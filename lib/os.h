#pragma once

#ifdef WIN32
    #include <Winsock2.h>
    #include <windows.h>
#else
    typedef int SOCKET;
    typedef int HANDLE;
#endif

namespace Os {
    bool set_blocking_mode(SOCKET socket, bool is_blocking);
    void close_socket(SOCKET socket);
    void close_epoll(HANDLE handle);
}
