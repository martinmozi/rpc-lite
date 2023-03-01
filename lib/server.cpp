#include "server.h"
#include "os.h"
#include <set>
#include <iostream>

#ifdef WIN32
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <string.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <sys/epoll.h>
	#include <arpa/inet.h>
	#include <netinet/tcp.h>
	#include <errno.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <signal.h>
	#include <sys/signalfd.h>
#endif

#define MAX_MSG_SIZE 8192 + sizeof(int)
#define MAX_CONNECTIONS 1000

namespace {
	void enable_keepalive(int sock) {
		int yes = 1;
		setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (const char*)&yes, sizeof(int));
#ifndef WIN32
		int idle = 1;
		setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(int));

		int interval = 1;
		setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(int));

		int maxpkt = 10;
		setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &maxpkt, sizeof(int));
#endif
	}

	std::atomic<int> gSignalFd(-1);

#ifdef WIN32
	void runServer(SOCKET listen_sock, RpcLite::Balancer& balancer) {
	}
#else
	// register events of fd to epfd
	void epoll_ctl_add(int epfd, int fd, uint32_t events)
	{
		struct epoll_event ev;
		ev.events = events;
		ev.data.fd = fd;
		if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
			perror("epoll_ctl()\n");
			exit(1);
		}
	}

	void initSignals(int epollHandle) {
		sigset_t mask;
		sigemptyset(&mask);
		sigaddset(&mask, SIGTERM);
		sigaddset(&mask, SIGINT);
		sigprocmask(SIG_BLOCK, &mask, 0);
		int signalFd = signalfd(-1, &mask, 0);

		epoll_event event;
		event.data.fd = signalFd;
		event.events = EPOLLIN;
		epoll_ctl(epollHandle, EPOLL_CTL_ADD, signalFd, &event);
		gSignalFd = signalFd;
	}

	void runServer(SOCKET listen_sock, RpcLite::Balancer& balancer) {
		std::set<int> clientSockets;
		char buf[MAX_MSG_SIZE];
		struct sockaddr_in cli_addr;
		struct epoll_event events[MAX_CONNECTIONS];
		int epollHandle = epoll_create(1);
		initSignals(epollHandle);
		epoll_ctl_add(epollHandle, listen_sock, EPOLLIN | EPOLLOUT | EPOLLET);
		socklen_t socklen = sizeof(cli_addr);

		for (;;) {
			int nfds = epoll_wait(epollHandle, events, MAX_CONNECTIONS, -1);
			for (int i = 0; i < nfds; i++) {
				if (events[i].data.fd == gSignalFd) {
					goto finish;
				}
				else if (events[i].data.fd == listen_sock) {
					// handle new connection
					int conn_sock = accept(listen_sock, (struct sockaddr*)&cli_addr, &socklen);
					enable_keepalive(conn_sock);
					inet_ntop(AF_INET, (char*)&(cli_addr.sin_addr), buf, sizeof(cli_addr));
					printf("[+] connected with %s:%d\n", buf, ntohs(cli_addr.sin_port));

					Os::set_blocking_mode(conn_sock, false);
					epoll_ctl_add(epollHandle, conn_sock, EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP);
					clientSockets.insert(conn_sock);
				}
				else if (events[i].events & EPOLLIN) {
					// handle EPOLLIN event
					memset(buf, 0, sizeof(buf));
					int n = ::recv(events[i].data.fd, buf + sizeof(int), sizeof(buf) - sizeof(int), 0);
					if (n > 0 /* || errno == EAGAIN */) {
						printf("[+] data: %s\n", buf + sizeof(int));
						*(int*)buf = events[i].data.fd; // first bytes are for identifying socket for answer
						balancer.enqueue(std::move(std::string(buf, n + sizeof(int))));
					}
				}
				else {
					printf("[+] unexpected\n");
				}
				// check if the connection is closing
				if (events[i].events & (EPOLLRDHUP | EPOLLHUP)) {
					printf("[+] connection closed\n");
					epoll_ctl(epollHandle, EPOLL_CTL_DEL, events[i].data.fd, NULL);
					close(events[i].data.fd);
					clientSockets.erase(events[i].data.fd);
					continue;
				}
			}
		}

	finish:
		::close(epollHandle);
		for (int clientSock : clientSockets) {
			::close(clientSock);
		}
	}
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<RpcLite::IServer> RpcLite::createServer(int port, int threadCount) {
    return std::make_unique<RpcLite::Server>(port, threadCount);
}

RpcLite::Server::Server(int port, int threadCount)
: _port(port)
, _threadCount(threadCount) 
, _balancer(threadCount) {
}

RpcLite::Server::~Server() {
}

bool RpcLite::Server::waitForData(int threadIndex, int& clientSocket, std::string& data) {
	// toto sa bude volat pre balancer a dany thread ma aj svoju queue
	std::string method = _balancer.dequeue(threadIndex);
	if (method.empty()) {
		return false;
	}

	clientSocket = *(const int*)method.c_str();
	data = std::string(method.c_str() + sizeof(int), method.size() - sizeof(int));
	return true;
}

void RpcLite::Server::send(int clientSocket, const std::string& data) {
#ifdef WIN32
	::send(clientSocket, data.c_str(), (int)data.size(), 0); // client doesn't have to exist
#else
	::send(clientSocket, data.c_str(), data.size(), 0);
#endif
}

void RpcLite::Server::run(RpcLite::IWorker&& iWorker) {
	struct sockaddr_in srv_addr;
	memset(&srv_addr, 0, sizeof(struct sockaddr_in));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = INADDR_ANY;
	srv_addr.sin_port = htons(_port);

	int optval = 1;
	SOCKET listen_sock = ::socket(AF_INET, SOCK_STREAM, 0);
	::setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval));
	::bind(listen_sock, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
	Os::set_blocking_mode(listen_sock, false);
	::listen(listen_sock, MAX_CONNECTIONS);
	_balancer.start(iWorker, *this);

	runServer(listen_sock, _balancer);
	Os::close_socket(listen_sock);
	_balancer.stop();
}

void RpcLite::Server::stop() {
#ifdef WIN32
	// some stop command
#else
	kill(getpid(), SIGTERM);
#endif
}
