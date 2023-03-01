#include "os.h"
#ifdef WIN32
#else
    #include <unistd.h>
    #include <fcntl.h>
#endif

bool Os::set_blocking_mode(SOCKET socket, bool is_blocking) {
#ifdef WIN32
    u_long non_blocking = is_blocking ? 0 : 1;
    return (NO_ERROR == ioctlsocket(socket, FIONBIO, &non_blocking));
#else
    const int flags = fcntl(socket, F_GETFL, 0);
    if ((flags & O_NONBLOCK) && !is_blocking) { return true; } // already in non blocking mode
    if (!(flags & O_NONBLOCK) && is_blocking) { return true; } // already in blocking mode
    return (0 == fcntl(socket, F_SETFL, is_blocking ? flags ^ O_NONBLOCK : flags | O_NONBLOCK));
#endif
}

void Os::close_socket(SOCKET socket) {
#ifdef WIN32
    ::closesocket(socket);
#else
    ::close(socket);
#endif
}