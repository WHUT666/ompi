/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 *
 * Winsock <-> POSIX socket glue for the native Windows build.
 */

#include "opal_config.h"

#ifdef _WIN32

#    include "opal/win32/opal_win32.h"

#    include <ws2tcpip.h>
#    include <mswsock.h>
#    include <mstcpip.h>
#    include <iphlpapi.h>
#    include <netioapi.h>

#    pragma comment(lib, "ws2_32.lib")
#    pragma comment(lib, "iphlpapi.lib")

/* undef the wrappers -- this file *implements* them and must call the
 * real Winsock entry points */
#    undef socket
#    undef socketpair
#    undef bind
#    undef connect
#    undef listen
#    undef accept
#    undef getsockname
#    undef getpeername
#    undef setsockopt
#    undef getsockopt
#    undef recv
#    undef recvfrom
#    undef send
#    undef sendto
#    undef sendmsg
#    undef recvmsg
#    undef shutdown
#    undef poll
#    undef select
#    undef close
#    undef gethostname
#    undef gethostbyname
#    undef gethostbyaddr
#    undef getifaddrs
#    undef freeifaddrs
#    undef if_nametoindex
#    undef if_indextoname
#    undef if_nameindex
#    undef if_freenameindex
#    undef inet_aton
#    undef inet_pton
#    undef inet_ntop

/* ------------------------------------------------------------------ */
/* socket fd registry: CRT fds and SOCKETs share the integer space,    */
/* so track which fds are sockets to route close() correctly.          */
/* ------------------------------------------------------------------ */

static CRITICAL_SECTION sock_reg_lock;
static volatile LONG sock_reg_init_done = 0;
static int *sock_reg_fds = NULL;
static size_t sock_reg_count = 0;
static size_t sock_reg_cap = 0;

static void sock_reg_lock_init(void)
{
    if (0 == InterlockedCompareExchange(&sock_reg_init_done, 1, 0)) {
        InitializeCriticalSection(&sock_reg_lock);
        InterlockedExchange(&sock_reg_init_done, 2);
    }
    while (1 == sock_reg_init_done) {
        Sleep(0);
    }
}

static void sock_reg_add(int fd)
{
    sock_reg_lock_init();
    EnterCriticalSection(&sock_reg_lock);
    if (sock_reg_count == sock_reg_cap) {
        size_t ncap = sock_reg_cap ? sock_reg_cap * 2 : 64;
        int *nfds = realloc(sock_reg_fds, ncap * sizeof(int));
        if (NULL == nfds) {
            LeaveCriticalSection(&sock_reg_lock);
            return;
        }
        sock_reg_fds = nfds;
        sock_reg_cap = ncap;
    }
    sock_reg_fds[sock_reg_count++] = fd;
    LeaveCriticalSection(&sock_reg_lock);
}

OPAL_WIN32_DECLSPEC int opal_win32_is_socket(int fd)
{
    size_t i;
    int found = 0;
    if (fd < 0) {
        return 0;
    }
    sock_reg_lock_init();
    EnterCriticalSection(&sock_reg_lock);
    for (i = 0; i < sock_reg_count; i++) {
        if (sock_reg_fds[i] == fd) {
            found = 1;
            break;
        }
    }
    LeaveCriticalSection(&sock_reg_lock);
    return found;
}

static void sock_reg_remove(int fd)
{
    size_t i;
    sock_reg_lock_init();
    EnterCriticalSection(&sock_reg_lock);
    for (i = 0; i < sock_reg_count; i++) {
        if (sock_reg_fds[i] == fd) {
            sock_reg_fds[i] = sock_reg_fds[--sock_reg_count];
            break;
        }
    }
    LeaveCriticalSection(&sock_reg_lock);
}

/* real definition of the WSA->errno mapper; consumers see it through
 * opal/opal_socket_errno.h's OPAL_DECLSPEC declaration. */
OPAL_WIN32_DECLSPEC int opal_win32_socket_errno(void)
{
    int wsaerr = WSAGetLastError();
    switch (wsaerr) {
    case 0:
        return 0;
    case WSAEINTR:
        return EINTR;
    case WSAEBADF:
        return EBADF;
    case WSAEACCES:
        return EACCES;
    case WSAEFAULT:
        return EFAULT;
    case WSAEINVAL:
        return EINVAL;
    case WSAEMFILE:
        return EMFILE;
    case WSAEWOULDBLOCK:
        return EWOULDBLOCK;
    case WSAEINPROGRESS:
        return EINPROGRESS;
    case WSAEALREADY:
        return EALREADY;
    case WSAENOTSOCK:
        return ENOTSOCK;
    case WSAEDESTADDRREQ:
        return EDESTADDRREQ;
    case WSAEMSGSIZE:
        return EMSGSIZE;
    case WSAEPROTOTYPE:
        return EPROTOTYPE;
    case WSAENOPROTOOPT:
        return ENOPROTOOPT;
    case WSAEPROTONOSUPPORT:
        return EPROTONOSUPPORT;
    case WSAESOCKTNOSUPPORT:
        return ESOCKTNOSUPPORT;
    case WSAEOPNOTSUPP:
        return EOPNOTSUPP;
    case WSAEPFNOSUPPORT:
        return EPFNOSUPPORT;
    case WSAEAFNOSUPPORT:
        return EAFNOSUPPORT;
    case WSAEADDRINUSE:
        return EADDRINUSE;
    case WSAEADDRNOTAVAIL:
        return EADDRNOTAVAIL;
    case WSAENETDOWN:
        return ENETDOWN;
    case WSAENETUNREACH:
        return ENETUNREACH;
    case WSAENETRESET:
        return ENETRESET;
    case WSAECONNABORTED:
        return ECONNABORTED;
    case WSAECONNRESET:
        return ECONNRESET;
    case WSAENOBUFS:
        return ENOBUFS;
    case WSAEISCONN:
        return EISCONN;
    case WSAENOTCONN:
        return ENOTCONN;
    case WSAESHUTDOWN:
        return ESHUTDOWN;
    case WSAETOOMANYREFS:
        return ETOOMANYREFS;
    case WSAETIMEDOUT:
        return ETIMEDOUT;
    case WSAECONNREFUSED:
        return ECONNREFUSED;
    case WSAELOOP:
        return ELOOP;
    case WSAENAMETOOLONG:
        return ENAMETOOLONG;
    case WSAEHOSTDOWN:
        return EHOSTDOWN;
    case WSAEHOSTUNREACH:
        return EHOSTUNREACH;
    case WSAENOTEMPTY:
        return ENOTEMPTY;
    case WSAEPROCLIM:
        return EPROCLIM;
    case WSAEUSERS:
        return EUSERS;
    case WSAEDQUOT:
        return EDQUOT;
    case WSAESTALE:
        return ESTALE;
    case WSAEREMOTE:
        return EREMOTE;
    default:
        return EIO;
    }
}

static int sock_error_ret(void)
{
    int e = opal_win32_socket_errno();
    errno = e;
    return -1;
}

OPAL_WIN32_DECLSPEC int opal_win32_socket_startup(void)
{
    return opal_win32_socket_init();
}

/* ------------------------------------------------------------------ */
/* wrappers: int fds in POSIX code <-> SOCKET (uintptr)                */
/* ------------------------------------------------------------------ */

OPAL_WIN32_DECLSPEC int opal_win32_socket(int domain, int type, int protocol)
{
    SOCKET s;
    if (0 != opal_win32_socket_init()) {
        errno = EACCES;
        return -1;
    }
    if (AF_UNIX == domain) {
        /* Windows 10 supports AF_UNIX; keep it enabled. */
        s = socket(domain, type, protocol);
    } else {
        s = WSASocketW(domain, type, protocol, NULL, 0, WSA_FLAG_OVERLAPPED);
    }
    if (INVALID_SOCKET == s) {
        return sock_error_ret();
    }
    sock_reg_add((int) s);
    return (int) s;
}

OPAL_WIN32_DECLSPEC int opal_win32_close(int fd)
{
    if (fd < 0) {
        errno = EBADF;
        return -1;
    }
    if (opal_win32_is_socket(fd)) {
        sock_reg_remove(fd);
        if (0 != closesocket((SOCKET) fd)) {
            return sock_error_ret();
        }
        return 0;
    }
    return _close(fd);
}

OPAL_WIN32_DECLSPEC ssize_t opal_win32_read(int fd, void *buf, size_t count)
{
    if (opal_win32_is_socket(fd)) {
        int rc = recv((SOCKET) fd, (char *) buf, (int) count, 0);
        if (SOCKET_ERROR == rc) {
            return sock_error_ret();
        }
        /* recv() returns 0 on orderly shutdown, matching read() EOF */
        return (ssize_t) rc;
    }
    if (opal_win32_fd_is_nonblocking(fd)) {
        /* Anonymous pipes cannot be put in a nonblocking mode; emulate
         * O_NONBLOCK with PeekNamedPipe so a poll-driven reader (iof's
         * evtimer pump) never blocks the progress thread on an empty
         * pipe. */
        HANDLE h = (HANDLE) _get_osfhandle(fd);
        DWORD avail = 0;
        if (INVALID_HANDLE_VALUE == h || NULL == h) {
            errno = EBADF;
            return -1;
        }
        if (FILE_TYPE_PIPE == GetFileType(h)) {
            if (!PeekNamedPipe(h, NULL, 0, NULL, &avail, NULL)) {
                DWORD gle = GetLastError();
                if (ERROR_BROKEN_PIPE == gle || ERROR_PIPE_NOT_CONNECTED == gle
                    || ERROR_NO_DATA == gle) {
                    /* writer closed: EOF */
                    return 0;
                }
                errno = EBADF;
                return -1;
            }
            if (0 == avail) {
                errno = EAGAIN;
                return -1;
            }
            if (count > (size_t) avail) {
                count = (size_t) avail;
            }
        }
    }
    return _read(fd, buf, (unsigned int) count);
}

OPAL_WIN32_DECLSPEC ssize_t opal_win32_write(int fd, const void *buf, size_t count)
{
    if (opal_win32_is_socket(fd)) {
        int rc = send((SOCKET) fd, (const char *) buf, (int) count, 0);
        if (SOCKET_ERROR == rc) {
            return sock_error_ret();
        }
        return (ssize_t) rc;
    }
    return _write(fd, buf, (unsigned int) count);
}

OPAL_WIN32_DECLSPEC int opal_win32_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    if (0 != bind((SOCKET) sockfd, addr, addrlen)) {
        return sock_error_ret();
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int opal_win32_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    if (0 != connect((SOCKET) sockfd, addr, addrlen)) {
        return sock_error_ret();
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int opal_win32_listen(int sockfd, int backlog)
{
    if (0 != listen((SOCKET) sockfd, backlog)) {
        return sock_error_ret();
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int opal_win32_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int alen = addrlen ? (int) *addrlen : 0;
    SOCKET s = accept((SOCKET) sockfd, addr, addrlen ? &alen : NULL);
    if (INVALID_SOCKET == s) {
        return sock_error_ret();
    }
    if (addrlen) {
        *addrlen = (socklen_t) alen;
    }
    sock_reg_add((int) s);
    return (int) s;
}

OPAL_WIN32_DECLSPEC int opal_win32_getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int alen = (int) *addrlen;
    if (0 != getsockname((SOCKET) sockfd, addr, &alen)) {
        return sock_error_ret();
    }
    *addrlen = (socklen_t) alen;
    return 0;
}

OPAL_WIN32_DECLSPEC int opal_win32_getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int alen = (int) *addrlen;
    if (0 != getpeername((SOCKET) sockfd, addr, &alen)) {
        return sock_error_ret();
    }
    *addrlen = (socklen_t) alen;
    return 0;
}

OPAL_WIN32_DECLSPEC int opal_win32_setsockopt(int sockfd, int level, int optname, const void *optval,
                          socklen_t optlen)
{
    int wsal = (SOL_SOCKET == level) ? SOL_SOCKET : level;
    int rc;
    if (SO_PEERCRED == optname || SO_PASSCRED == optname || SO_BINDTODEVICE == optname) {
        errno = EOPNOTSUPP;
        return -1;
    }
    /* normalize: POSIX callers pass struct timeval for timeouts */
    if (SOL_SOCKET == level && (SO_RCVTIMEO == optname || SO_SNDTIMEO == optname)
        && sizeof(struct timeval) == optlen) {
        struct timeval *tv = (struct timeval *) optval;
        DWORD ms = (DWORD) (tv->tv_sec * 1000 + tv->tv_usec / 1000);
        rc = setsockopt((SOCKET) sockfd, wsal, optname, (const char *) &ms, sizeof(ms));
    } else {
        rc = setsockopt((SOCKET) sockfd, wsal, optname, (const char *) optval, (int) optlen);
    }
    if (0 != rc) {
        return sock_error_ret();
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int opal_win32_getsockopt(int sockfd, int level, int optname, void *optval,
                          socklen_t *optlen)
{
    int len = optlen ? (int) *optlen : 0;
    if (SO_PEERCRED == optname || SO_PASSCRED == optname) {
        errno = EOPNOTSUPP;
        return -1;
    }
    if (0 != getsockopt((SOCKET) sockfd, level, optname, (char *) optval, &len)) {
        return sock_error_ret();
    }
    if (optlen) {
        *optlen = (socklen_t) len;
    }
    return 0;
}

OPAL_WIN32_DECLSPEC ssize_t opal_win32_recv(int sockfd, void *buf, size_t len, int flags)
{
    int rc = recv((SOCKET) sockfd, (char *) buf, (int) len, flags);
    if (SOCKET_ERROR == rc) {
        return sock_error_ret();
    }
    return rc;
}

OPAL_WIN32_DECLSPEC ssize_t opal_win32_recvfrom(int sockfd, void *buf, size_t len, int flags,
                            struct sockaddr *src_addr, socklen_t *addrlen)
{
    int alen = addrlen ? (int) *addrlen : 0;
    int rc = recvfrom((SOCKET) sockfd, (char *) buf, (int) len, flags, src_addr,
                      addrlen ? &alen : NULL);
    if (SOCKET_ERROR == rc) {
        return sock_error_ret();
    }
    if (addrlen) {
        *addrlen = (socklen_t) alen;
    }
    return rc;
}

OPAL_WIN32_DECLSPEC ssize_t opal_win32_send(int sockfd, const void *buf, size_t len, int flags)
{
    int rc = send((SOCKET) sockfd, (const char *) buf, (int) len, flags);
    if (SOCKET_ERROR == rc) {
        return sock_error_ret();
    }
    return rc;
}

OPAL_WIN32_DECLSPEC ssize_t opal_win32_sendto(int sockfd, const void *buf, size_t len, int flags,
                          const struct sockaddr *dest_addr, socklen_t addrlen)
{
    int rc = sendto((SOCKET) sockfd, (const char *) buf, (int) len, flags,
                    dest_addr, (int) addrlen);
    if (SOCKET_ERROR == rc) {
        return sock_error_ret();
    }
    return rc;
}

OPAL_WIN32_DECLSPEC int opal_win32_shutdown(int sockfd, int how)
{
    if (0 != shutdown((SOCKET) sockfd, how)) {
        return sock_error_ret();
    }
    return 0;
}

static WSABUF *iov_to_wsabuf(const struct iovec *iov, int iovcnt)
{
    WSABUF *bufs = malloc(sizeof(WSABUF) * (size_t) iovcnt);
    int i;
    if (NULL == bufs) {
        return NULL;
    }
    for (i = 0; i < iovcnt; i++) {
        bufs[i].buf = (CHAR *) iov[i].iov_base;
        bufs[i].len = (ULONG) iov[i].iov_len;
    }
    return bufs;
}

OPAL_WIN32_DECLSPEC ssize_t opal_win32_sendmsg(int sockfd, const struct msghdr *msg, int flags)
{
    WSABUF *bufs;
    DWORD sent = 0;
    int rc;
    (void) flags;
    if (NULL != msg->msg_control && msg->msg_controllen > 0) {
        /* ancillary data (fd passing) is not supported on Windows */
        errno = EOPNOTSUPP;
        return -1;
    }
    bufs = iov_to_wsabuf(msg->msg_iov, msg->msg_iovlen);
    if (NULL == bufs) {
        errno = ENOMEM;
        return -1;
    }
    rc = WSASendTo((SOCKET) sockfd, bufs, (DWORD) msg->msg_iovlen, &sent, 0,
                   msg->msg_name, (int) msg->msg_namelen, NULL, NULL);
    free(bufs);
    if (0 != rc) {
        return sock_error_ret();
    }
    return (ssize_t) sent;
}

OPAL_WIN32_DECLSPEC ssize_t opal_win32_recvmsg(int sockfd, struct msghdr *msg, int flags)
{
    WSABUF *bufs;
    DWORD recvd = 0;
    DWORD wflags = (DWORD) flags;
    int rc;
    if (NULL != msg->msg_control && msg->msg_controllen > 0) {
        /* no fd passing support */
        msg->msg_controllen = 0;
    }
    bufs = iov_to_wsabuf(msg->msg_iov, msg->msg_iovlen);
    if (NULL == bufs) {
        errno = ENOMEM;
        return -1;
    }
    rc = WSARecvFrom((SOCKET) sockfd, bufs, (DWORD) msg->msg_iovlen, &recvd, &wflags,
                     msg->msg_name, &msg->msg_namelen, NULL, NULL);
    free(bufs);
    if (0 != rc) {
        return sock_error_ret();
    }
    msg->msg_flags = (int) wflags;
    return (ssize_t) recvd;
}

/* ------------------------------------------------------------------ */
/* socketpair() -- loopback TCP pair                                   */
/* ------------------------------------------------------------------ */

OPAL_WIN32_DECLSPEC int opal_win32_socketpair(int domain, int type, int protocol, int sv[2])
{
    SOCKET listener = INVALID_SOCKET;
    SOCKET a = INVALID_SOCKET, b = INVALID_SOCKET;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int rc = -1;

    (void) domain;
    (void) protocol;
    opal_win32_socket_init();

    listener = socket(AF_INET, type, protocol);
    if (INVALID_SOCKET == listener) {
        goto out;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;
    if (0 != bind(listener, (struct sockaddr *) &addr, sizeof(addr))) {
        goto out;
    }
    {
        int alen = (int) sizeof(addr);
        if (0 != getsockname(listener, (struct sockaddr *) &addr, &alen)) {
            goto out;
        }
    }
    if (0 != listen(listener, 1)) {
        goto out;
    }
    a = socket(AF_INET, type, protocol);
    if (INVALID_SOCKET == a) {
        goto out;
    }
    if (0 != connect(a, (struct sockaddr *) &addr, sizeof(addr))) {
        goto out;
    }
    b = accept(listener, NULL, NULL);
    if (INVALID_SOCKET == b) {
        goto out;
    }
    sv[0] = (int) a;
    sv[1] = (int) b;
    sock_reg_add((int) a);
    sock_reg_add((int) b);
    rc = 0;
out:
    if (INVALID_SOCKET != listener) {
        closesocket(listener);
    }
    if (0 != rc) {
        int e = WSAGetLastError();
        if (INVALID_SOCKET != a) {
            closesocket(a);
        }
        if (INVALID_SOCKET != b) {
            closesocket(b);
        }
        WSASetLastError(e);
        errno = opal_win32_socket_errno();
        return -1;
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* poll() over select()                                                */
/* ------------------------------------------------------------------ */

OPAL_WIN32_DECLSPEC int opal_win32_poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
    fd_set rfds, wfds, efds;
    nfds_t i;
    int rc, nready = 0;
    SOCKET maxfd = 0;
    struct timeval tv, *ptv = NULL;

    opal_win32_socket_init();
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_ZERO(&efds);
    for (i = 0; i < nfds; i++) {
        if (fds[i].fd < 0) {
            continue;
        }
        if (fds[i].events & (POLLIN | POLLPRI | POLLRDNORM | POLLRDBAND)) {
            FD_SET((SOCKET) fds[i].fd, &rfds);
        }
        if (fds[i].events & (POLLOUT | POLLWRNORM | POLLWRBAND)) {
            FD_SET((SOCKET) fds[i].fd, &wfds);
        }
        FD_SET((SOCKET) fds[i].fd, &efds);
        if ((SOCKET) fds[i].fd > maxfd) {
            maxfd = (SOCKET) fds[i].fd;
        }
        fds[i].revents = 0;
    }
    if (timeout >= 0) {
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
        ptv = &tv;
    }
    rc = select((int) maxfd + 1, &rfds, &wfds, &efds, ptv);
    if (rc < 0) {
        return sock_error_ret();
    }
    if (0 == rc) {
        return 0;
    }
    for (i = 0; i < nfds; i++) {
        short rev = 0;
        if (fds[i].fd < 0) {
            continue;
        }
        if (FD_ISSET((SOCKET) fds[i].fd, &rfds)) {
            rev |= POLLIN;
        }
        if (FD_ISSET((SOCKET) fds[i].fd, &wfds)) {
            rev |= POLLOUT;
        }
        if (FD_ISSET((SOCKET) fds[i].fd, &efds)) {
            rev |= POLLERR | POLLHUP;
        }
        if (rev) {
            /* detect orderly shutdown / reset as POLLHUP */
            if (fds[i].events & POLLIN) {
                char c;
                int r = recv((SOCKET) fds[i].fd, &c, 1, MSG_PEEK);
                if (0 == r) {
                    rev |= POLLHUP;
                } else if (r < 0 && WSAGetLastError() == WSAECONNRESET) {
                    rev |= POLLHUP | POLLERR;
                }
            }
            fds[i].revents = rev;
            nready++;
        }
    }
    return nready;
}

/* ------------------------------------------------------------------ */
/* misc posix socket extras                                            */
/* ------------------------------------------------------------------ */

OPAL_WIN32_DECLSPEC int opal_win32_inet_aton(const char *cp, struct in_addr *inp)
{
    struct in_addr a;
    int rc = inet_pton(AF_INET, cp, &a);
    if (1 != rc) {
        return 0;
    }
    inp->s_addr = a.s_addr;
    return 1;
}

OPAL_WIN32_DECLSPEC const char *opal_win32_inet_ntop(int af, const void *src, char *dst, socklen_t size)
{
    if (AF_INET == af) {
        DWORD len = (DWORD) size;
        struct sockaddr_in sa;
        memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        memcpy(&sa.sin_addr, src, sizeof(struct in_addr));
        if (0 != WSAAddressToStringA((LPSOCKADDR) &sa, sizeof(sa), NULL, dst, &len)) {
            return NULL;
        }
        return dst;
    }
    if (AF_INET6 == af) {
        DWORD len = (DWORD) size;
        struct sockaddr_in6 sa6;
        memset(&sa6, 0, sizeof(sa6));
        sa6.sin6_family = AF_INET6;
        memcpy(&sa6.sin6_addr, src, sizeof(struct in6_addr));
        if (0 != WSAAddressToStringA((LPSOCKADDR) &sa6, sizeof(sa6), NULL, dst, &len)) {
            return NULL;
        }
        return dst;
    }
    errno = EAFNOSUPPORT;
    return NULL;
}

OPAL_WIN32_DECLSPEC int opal_win32_inet_pton(int af, const char *src, void *dst)
{
    INT rc;
    opal_win32_socket_init();
    rc = InetPtonA(af, src, dst);
    return rc;
}

OPAL_WIN32_DECLSPEC struct hostent *opal_win32_gethostbyname(const char *name)
{
    opal_win32_socket_init();
    return gethostbyname(name);
}

OPAL_WIN32_DECLSPEC struct hostent *opal_win32_gethostbyaddr(const void *addr, int len, int type)
{
    opal_win32_socket_init();
    return gethostbyaddr((const char *) addr, len, type);
}

OPAL_WIN32_DECLSPEC int opal_win32_gethostname(char *name, size_t len)
{
    DWORD sz = (DWORD) len;
    opal_win32_socket_init();
    if (0 == gethostname(name, (int) len)) {
        /* ensure NUL termination */
        name[len - 1] = '\0';
        return 0;
    }
    return sock_error_ret();
}

/* ------------------------------------------------------------------ */
/* getifaddrs() via GetAdaptersAddresses()                             */
/* ------------------------------------------------------------------ */

static int sockaddr_from_unicast(struct sockaddr **dst, SOCKET_ADDRESS *src)
{
    size_t len = (size_t) src->iSockaddrLength;
    *dst = malloc(len);
    if (NULL == *dst) {
        return -1;
    }
    memcpy(*dst, src->lpSockaddr, len);
    return 0;
}

static int sockaddr_from_prefix(struct sockaddr **dst, int family, UCHAR prefixlen)
{
    if (AF_INET == family) {
        struct sockaddr_in *sa = calloc(1, sizeof(*sa));
        if (NULL == sa) {
            return -1;
        }
        sa->sin_family = AF_INET;
        if (prefixlen > 32) {
            prefixlen = 32;
        }
        sa->sin_addr.s_addr = htonl(prefixlen ? (0xffffffffU << (32 - prefixlen)) : 0);
        *dst = (struct sockaddr *) sa;
        return 0;
    }
    if (AF_INET6 == family) {
        struct sockaddr_in6 *sa6 = calloc(1, sizeof(*sa6));
        int i;
        if (NULL == sa6) {
            return -1;
        }
        sa6->sin6_family = AF_INET6;
        if (prefixlen > 128) {
            prefixlen = 128;
        }
        for (i = 0; i < 16 && prefixlen >= 8; i++, prefixlen -= 8) {
            sa6->sin6_addr.u.Byte[i] = 0xff;
        }
        if (prefixlen > 0 && prefixlen < 8 && i < 16) {
            sa6->sin6_addr.u.Byte[i] = (UCHAR) (0xff << (8 - prefixlen));
        }
        *dst = (struct sockaddr *) sa6;
        return 0;
    }
    *dst = NULL;
    return 0;
}

OPAL_WIN32_DECLSPEC int opal_win32_getifaddrs(struct ifaddrs **ifap)
{
    IP_ADAPTER_ADDRESSES *aa = NULL, *cur;
    IP_ADAPTER_UNICAST_ADDRESS *ua;
    ULONG buflen = 16384;
    ULONG rc;
    int tries;
    struct ifaddrs *list = NULL, **tail = &list;
    int err = 0;

    *ifap = NULL;
    for (tries = 0; tries < 3; tries++) {
        aa = malloc(buflen);
        if (NULL == aa) {
            errno = ENOMEM;
            return -1;
        }
        rc = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX
                                              | GAA_FLAG_SKIP_ANYCAST
                                              | GAA_FLAG_SKIP_MULTICAST
                                              | GAA_FLAG_SKIP_DNS_SERVER,
                                  NULL, aa, &buflen);
        if (ERROR_BUFFER_OVERFLOW == rc) {
            free(aa);
            aa = NULL;
            continue;
        }
        if (ERROR_SUCCESS == rc) {
            break;
        }
        free(aa);
        errno = EIO;
        return -1;
    }
    if (NULL == aa) {
        errno = ENOMEM;
        return -1;
    }

    for (cur = aa; cur; cur = cur->Next) {
        for (ua = cur->FirstUnicastAddress; ua; ua = ua->Next) {
            struct ifaddrs *ifa = calloc(1, sizeof(*ifa));
            if (NULL == ifa) {
                err = ENOMEM;
                goto out;
            }
            ifa->ifa_name = _strdup(cur->AdapterName);
            /* flags */
            if (cur->OperStatus == IfOperStatusUp) {
                ifa->ifa_flags |= IFF_UP | IFF_RUNNING;
            }
            if (cur->IfType == IF_TYPE_SOFTWARE_LOOPBACK) {
                ifa->ifa_flags |= IFF_LOOPBACK;
            }
            if (cur->IfType == IF_TYPE_PPP || cur->IfType == IF_TYPE_TUNNEL) {
                ifa->ifa_flags |= IFF_POINTOPOINT;
            }
            if (!(cur->Flags & IP_ADAPTER_NO_MULTICAST)) {
                ifa->ifa_flags |= IFF_MULTICAST;
            }
            if (sockaddr_from_unicast(&ifa->ifa_addr, &ua->Address)) {
                free(ifa->ifa_name);
                free(ifa);
                continue;
            }
            if (sockaddr_from_prefix(&ifa->ifa_netmask,
                                     ua->Address.lpSockaddr->sa_family,
                                     ua->OnLinkPrefixLength)) {
                free(ifa->ifa_addr);
                free(ifa->ifa_name);
                free(ifa);
                continue;
            }
            *tail = ifa;
            tail = &ifa->ifa_next;
        }
    }
    *ifap = list;
    free(aa);
    return 0;

out:
    free(aa);
    opal_win32_freeifaddrs(list);
    errno = err;
    return -1;
}

OPAL_WIN32_DECLSPEC void opal_win32_freeifaddrs(struct ifaddrs *ifa)
{
    while (ifa) {
        struct ifaddrs *next = ifa->ifa_next;
        free(ifa->ifa_name);
        free(ifa->ifa_addr);
        free(ifa->ifa_netmask);
        free(ifa->ifa_ifu.ifu_broadaddr);
        free(ifa);
        ifa = next;
    }
}

/* ------------------------------------------------------------------ */
/* if_nametoindex et al.                                               */
/* ------------------------------------------------------------------ */

OPAL_WIN32_DECLSPEC unsigned int opal_win32_if_nametoindex(const char *ifname)
{
    IP_ADAPTER_ADDRESSES *aa = NULL, *cur;
    ULONG buflen = 16384;
    ULONG rc;
    unsigned int idx = 0;
    int tries;

    for (tries = 0; tries < 3; tries++) {
        aa = malloc(buflen);
        if (NULL == aa) {
            return 0;
        }
        rc = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, aa,
                                  &buflen);
        if (ERROR_BUFFER_OVERFLOW == rc) {
            free(aa);
            aa = NULL;
            continue;
        }
        break;
    }
    if (NULL == aa) {
        return 0;
    }
    for (cur = aa; cur; cur = cur->Next) {
        char namebuf[256];
        /* AdapterName is the GUID-style name; FriendlyName is the
         * human name.  Accept either. */
        WideCharToMultiByte(CP_UTF8, 0, cur->FriendlyName, -1, namebuf,
                            sizeof(namebuf), NULL, NULL);
        if ((0 == _stricmp(namebuf, ifname))
            || (0 == _stricmp(cur->AdapterName, ifname))) {
            idx = (unsigned int) cur->IfIndex;
            break;
        }
    }
    free(aa);
    return idx;
}

OPAL_WIN32_DECLSPEC char *opal_win32_if_indextoname(unsigned int ifindex, char *ifname)
{
    IP_ADAPTER_ADDRESSES *aa = NULL, *cur;
    ULONG buflen = 16384;
    ULONG rc;
    int tries;
    char *ret = NULL;

    for (tries = 0; tries < 3; tries++) {
        aa = malloc(buflen);
        if (NULL == aa) {
            return NULL;
        }
        rc = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, aa,
                                  &buflen);
        if (ERROR_BUFFER_OVERFLOW == rc) {
            free(aa);
            aa = NULL;
            continue;
        }
        break;
    }
    if (NULL == aa) {
        return NULL;
    }
    for (cur = aa; cur; cur = cur->Next) {
        if (ifindex == cur->IfIndex) {
            strncpy(ifname, cur->AdapterName, IFNAMSIZ - 1);
            ifname[IFNAMSIZ - 1] = '\0';
            ret = ifname;
            break;
        }
    }
    free(aa);
    return ret;
}

OPAL_WIN32_DECLSPEC struct if_nameindex *opal_win32_if_nameindex(void)
{
    IP_ADAPTER_ADDRESSES *aa = NULL, *cur;
    ULONG buflen = 16384;
    ULONG rc;
    int tries;
    size_t count = 0, i = 0;
    struct if_nameindex *list;

    for (tries = 0; tries < 3; tries++) {
        aa = malloc(buflen);
        if (NULL == aa) {
            return NULL;
        }
        rc = GetAdaptersAddresses(AF_UNSPEC, 0, NULL, aa, &buflen);
        if (ERROR_BUFFER_OVERFLOW == rc) {
            free(aa);
            aa = NULL;
            continue;
        }
        break;
    }
    if (NULL == aa) {
        return NULL;
    }
    for (cur = aa; cur; cur = cur->Next) {
        count++;
    }
    list = calloc(count + 1, sizeof(*list));
    if (NULL == list) {
        free(aa);
        return NULL;
    }
    for (cur = aa; cur && i < count; cur = cur->Next) {
        list[i].if_index = (unsigned int) cur->IfIndex;
        list[i].if_name = _strdup(cur->AdapterName);
        i++;
    }
    free(aa);
    return list;
}

OPAL_WIN32_DECLSPEC void opal_win32_if_freenameindex(struct if_nameindex *ptr)
{
    size_t i;
    if (NULL == ptr) {
        return;
    }
    for (i = 0; ptr[i].if_name; i++) {
        free(ptr[i].if_name);
    }
    free(ptr);
}

#endif /* _WIN32 */
