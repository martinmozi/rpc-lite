#pragma once

#ifdef WIN32
    #include <Winsock2.h>
    #include <windows.h>
    typedef int socklen_t;
#else
    typedef int SOCKET;
#endif

namespace Os {
    bool set_blocking_mode(SOCKET socket, bool is_blocking);
    void close_socket(SOCKET socket);
}