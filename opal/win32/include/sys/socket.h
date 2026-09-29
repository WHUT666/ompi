/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 *
 * POSIX <sys/socket.h> replacement for the native Windows build.
 * Winsock2 supplies the bulk of the API; this header adds the missing
 * pieces (msghdr/cmsg/iovec glue, sendmsg/recvmsg, socketpair).
 */

#ifndef OPAL_WIN32_SYS_SOCKET_H
#define OPAL_WIN32_SYS_SOCKET_H

#include "opal_win32_common.h"
#include <sys/types.h>
#include <sys/uio.h>

struct pollfd;

#ifdef __cplusplus
extern "C" {
#endif

/* socklen_t and sa_family_t: Winsock APIs take int for length parameters
 * and use ADDRESS_FAMILY (USHORT) for address families; the MSVC SDK
 * defines neither type, even via ws2def.h, so declare both here. */
typedef int socklen_t;
typedef unsigned short sa_family_t;

/* shutdown() "how" constants */
#define SHUT_RD   SD_RECEIVE
#define SHUT_WR   SD_SEND
#define SHUT_RDWR SD_BOTH

/* POSIX MSG_* flags that Winsock lacks */
#ifndef MSG_OOB
#    define MSG_OOB 0x1
#endif
#ifndef MSG_PEEK
#    define MSG_PEEK 0x2
#endif
#ifndef MSG_DONTROUTE
#    define MSG_DONTROUTE 0x4
#endif
#ifndef MSG_CTRUNC
#    define MSG_CTRUNC 0x0100
#endif
#ifndef MSG_TRUNC
#    define MSG_TRUNC 0x0100
#endif
#ifndef MSG_WAITALL
#    define MSG_WAITALL 0x8
#endif
#ifndef MSG_NOSIGNAL
#    define MSG_NOSIGNAL 0 /* no SIGPIPE on Windows */
#endif
#ifndef MSG_EOR
#    define MSG_EOR 0x80
#endif
#ifndef MSG_DONTWAIT
#    define MSG_DONTWAIT 0 /* callers must set nonblocking explicitly */
#endif
#ifndef MSG_MORE
#    define MSG_MORE 0
#endif
#ifndef MSG_CONFIRM
#    define MSG_CONFIRM 0
#endif
#ifndef MSG_NBIO
#    define MSG_NBIO 0
#endif

#ifndef AF_UNIX
#    define AF_UNIX 1
#endif
#ifndef AF_LOCAL
#    define AF_LOCAL AF_UNIX
#endif
#ifndef PF_UNIX
#    define PF_UNIX AF_UNIX
#endif
#ifndef PF_LOCAL
#    define PF_LOCAL AF_LOCAL
#endif

/* SOL_* levels missing in Winsock */
#ifndef SOL_TCP
#    define SOL_TCP IPPROTO_TCP
#endif
#ifndef SOL_SOCKET_ONLY
#    define SOL_SOCKET_ONLY 0xffffffff
#endif

#ifndef SO_REUSEPORT
#    define SO_REUSEPORT 0x0200 /* not supported; map to nothing */
#endif
#ifndef SO_BINDTODEVICE
#    define SO_BINDTODEVICE 0xffff /* fail gracefully */
#endif
#ifndef SO_SNDBUFFORCE
#    define SO_SNDBUFFORCE SO_SNDBUF
#endif
#ifndef SO_RCVBUFFORCE
#    define SO_RCVBUFFORCE SO_RCVBUF
#endif
#ifndef SO_PEERCRED
#    define SO_PEERCRED 0x1000 /* unsupported on Windows */
#endif
#ifndef SO_PASSCRED
#    define SO_PASSCRED 0x1001
#endif
#ifndef SO_TIMESTAMP
/* same value as <mstcpip.h> so cmsg timestamp code behaves identically
 * whichever header provides it */
#    define SO_TIMESTAMP 0x300A
#endif
#ifndef SO_BUSY_POLL
#    define SO_BUSY_POLL 0x1003
#endif
#ifndef SO_LINGER_SEC
#    define SO_LINGER_SEC SO_LINGER
#endif

/* POSIX ancillary data */
#ifndef SCM_RIGHTS
#    define SCM_RIGHTS 0x01
#endif
#ifndef SCM_CREDENTIALS
#    define SCM_CREDENTIALS 0x02
#endif

/* POSIX msghdr layout.  Winsock provides struct cmsghdr and the
 * CMSG_* macros via ws2def.h (included through opal_win32_common.h),
 * so we only add msghdr plus the pieces the SDK does not give us. */
struct msghdr {
    void         *msg_name;
    socklen_t     msg_namelen;
    struct iovec *msg_iov;
    int           msg_iovlen;
    void         *msg_control;
    size_t        msg_controllen;
    int           msg_flags;
};

#ifndef CMSG_SPACE
#    ifndef CMSG_ALIGN
#        define CMSG_ALIGN(len) (((len) + sizeof(size_t) - 1) & ~(sizeof(size_t) - 1))
#    endif
#    define CMSG_SPACE(len) (CMSG_ALIGN(sizeof(struct cmsghdr)) + CMSG_ALIGN(len))
#    define CMSG_LEN(len)   (CMSG_ALIGN(sizeof(struct cmsghdr)) + (len))
#    define CMSG_FIRSTHDR(msg) \
        (((msg)->msg_controllen >= sizeof(struct cmsghdr)) ? (struct cmsghdr *) (msg)->msg_control \
                                                          : (struct cmsghdr *) NULL)
#    define CMSG_NXTHDR(msg, cmsg)                                                 \
        (((cmsg) == NULL)                                                          \
             ? CMSG_FIRSTHDR(msg)                                                  \
             : (((unsigned char *) (cmsg) + CMSG_ALIGN((cmsg)->cmsg_len)           \
                     + CMSG_ALIGN(sizeof(struct cmsghdr))                          \
                 > (unsigned char *) (msg)->msg_control + (msg)->msg_controllen)   \
                    ? NULL                                                         \
                    : (struct cmsghdr *) ((unsigned char *) (cmsg)                 \
                                          + CMSG_ALIGN((cmsg)->cmsg_len))))
#    define CMSG_DATA(cmsg) ((unsigned char *) (cmsg) + CMSG_ALIGN(sizeof(struct cmsghdr)))
#endif
#ifndef CMSG_OK
#    define CMSG_OK(msg, cmsg) 1
#endif

/* socket API glue */
#define socket        opal_win32_socket
#define socketpair    opal_win32_socketpair
/* NOTE: no macro for bind(): "bind" is used as a struct member name in
   mca_base_pvar.h / mca_base_event.h / mpool_memkind.h, so a macro would
   rewrite those tokens too.  Winsock bind() has a compatible signature;
   errors are retrieved via opal_socket_errno(). */
/* connect is function-like (not object-like) on purpose: PMIx's
 * pmix_server_module_t has a function-pointer member named "connect"
 * that hosts invoke as host_module.connect(...) -- an object-like
 * macro would rewrite those member tokens.  A function-like macro only
 * expands on real call syntax. */
#define connect(a,b,c) opal_win32_connect((a),(b),(c))
#define listen        opal_win32_listen
#define accept        opal_win32_accept
#define getsockname   opal_win32_getsockname
#define getpeername   opal_win32_getpeername
#define setsockopt    opal_win32_setsockopt
#define getsockopt    opal_win32_getsockopt
#define recv          opal_win32_recv
#define recvfrom      opal_win32_recvfrom
#define send          opal_win32_send
#define sendto        opal_win32_sendto
#define sendmsg       opal_win32_sendmsg
#define recvmsg       opal_win32_recvmsg
#define shutdown      opal_win32_shutdown
#define sockatmark(s) 0

OPAL_WIN32_DECLSPEC int opal_win32_socket(int domain, int type, int protocol);
OPAL_WIN32_DECLSPEC int opal_win32_socketpair(int domain, int type, int protocol, int sv[2]);
OPAL_WIN32_DECLSPEC int opal_win32_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
OPAL_WIN32_DECLSPEC int opal_win32_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
OPAL_WIN32_DECLSPEC int opal_win32_listen(int sockfd, int backlog);
OPAL_WIN32_DECLSPEC int opal_win32_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
OPAL_WIN32_DECLSPEC int opal_win32_getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
OPAL_WIN32_DECLSPEC int opal_win32_getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
OPAL_WIN32_DECLSPEC int opal_win32_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
OPAL_WIN32_DECLSPEC int opal_win32_getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
OPAL_WIN32_DECLSPEC ssize_t opal_win32_recv(int sockfd, void *buf, size_t len, int flags);
OPAL_WIN32_DECLSPEC ssize_t opal_win32_recvfrom(int sockfd, void *buf, size_t len, int flags,
                            struct sockaddr *src_addr, socklen_t *addrlen);
OPAL_WIN32_DECLSPEC ssize_t opal_win32_send(int sockfd, const void *buf, size_t len, int flags);
OPAL_WIN32_DECLSPEC ssize_t opal_win32_sendto(int sockfd, const void *buf, size_t len, int flags,
                          const struct sockaddr *dest_addr, socklen_t addrlen);
OPAL_WIN32_DECLSPEC ssize_t opal_win32_sendmsg(int sockfd, const struct msghdr *msg, int flags);
OPAL_WIN32_DECLSPEC ssize_t opal_win32_recvmsg(int sockfd, struct msghdr *msg, int flags);
OPAL_WIN32_DECLSPEC int opal_win32_shutdown(int sockfd, int how);

/* returns non-zero if fd refers to a socket (rather than a CRT fd) */
OPAL_WIN32_DECLSPEC int opal_win32_is_socket(int fd);
/* close() that handles both sockets and CRT fds -- all OMPI code that
 * closes a possibly-socket fd routes through this on Windows */
OPAL_WIN32_DECLSPEC int opal_win32_close(int fd);
OPAL_WIN32_DECLSPEC int opal_win32_poll(struct pollfd *fds, unsigned long nfds, int timeout);

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_SYS_SOCKET_H */
