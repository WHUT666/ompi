/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <netinet/in.h> replacement for the native Windows build.
 * Winsock2/ws2def provide most of it; this fills gaps.
 */
#ifndef OPAL_WIN32_NETINET_IN_H
#define OPAL_WIN32_NETINET_IN_H

#include "opal_win32_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;

#ifndef IPPROTO_IP
#    define IPPROTO_IP 0
#endif
#ifndef IPPROTO_ICMP
#    define IPPROTO_ICMP 1
#endif
#ifndef IPPROTO_IGMP
#    define IPPROTO_IGMP 2
#endif
#ifndef IPPROTO_IPV4
#    define IPPROTO_IPV4 4
#endif
#ifndef IPPROTO_TCP
#    define IPPROTO_TCP 6
#endif
#ifndef IPPROTO_UDP
#    define IPPROTO_UDP 17
#endif
#ifndef IPPROTO_IPV6
#    define IPPROTO_IPV6 41
#endif
#ifndef IPPROTO_ICMPV6
#    define IPPROTO_ICMPV6 58
#endif
#ifndef IPPROTO_RAW
#    define IPPROTO_RAW 255
#endif
#ifndef IPPROTO_HOPOPTS
#    define IPPROTO_HOPOPTS 0
#endif
#ifndef IPPROTO_ROUTING
#    define IPPROTO_ROUTING 43
#endif
#ifndef IPPROTO_FRAGMENT
#    define IPPROTO_FRAGMENT 44
#endif
#ifndef IPPROTO_NONE
#    define IPPROTO_NONE 59
#endif
#ifndef IPPROTO_DSTOPTS
#    define IPPROTO_DSTOPTS 60
#endif

#ifndef INADDR_LOOPBACK
#    define INADDR_LOOPBACK 0x7f000001
#endif
#ifndef INADDR_ANY
#    define INADDR_ANY 0x00000000
#endif
#ifndef INADDR_BROADCAST
#    define INADDR_BROADCAST 0xffffffff
#endif
#ifndef INADDR_NONE
#    define INADDR_NONE 0xffffffff
#endif
#ifndef IN_LOOPBACKNET
#    define IN_LOOPBACKNET 127
#endif
#ifndef INET_ADDRSTRLEN
#    define INET_ADDRSTRLEN 16
#endif
#ifndef INET6_ADDRSTRLEN
#    define INET6_ADDRSTRLEN 46
#endif

/* multicast request structures: ws2ipdef.h (via ws2tcpip.h in
 * opal_win32_common.h) already provides ip_mreq/ip_mreq_source and
 * IPV6_* options; only add the GNU-style ip_mreqn which it lacks */
#ifndef _WS2IPDEF_
struct ip_mreq {
    struct in_addr imr_multiaddr;
    struct in_addr imr_interface;
};
#endif
struct ip_mreqn {
    struct in_addr imr_multiaddr;
    struct in_addr imr_address;
    int            imr_ifindex;
};

#ifndef IN6_IS_ADDR_UNSPECIFIED
#    define IN6_IS_ADDR_UNSPECIFIED(a) (((const uint32_t *) (a))[0] == 0 && \
                                        ((const uint32_t *) (a))[1] == 0 && \
                                        ((const uint32_t *) (a))[2] == 0 && \
                                        ((const uint32_t *) (a))[3] == 0)
#endif
#ifndef IN6_IS_ADDR_LOOPBACK
#    define IN6_IS_ADDR_LOOPBACK(a) (((const uint32_t *) (a))[0] == 0 && \
                                     ((const uint32_t *) (a))[1] == 0 && \
                                     ((const uint32_t *) (a))[2] == 0 && \
                                     ((const uint32_t *) (a))[3] == 0x01000000)
#endif
#ifndef IN6_IS_ADDR_MULTICAST
#    define IN6_IS_ADDR_MULTICAST(a) (((const uint8_t *) (a))[0] == 0xff)
#endif
#ifndef IN6_IS_ADDR_LINKLOCAL
#    define IN6_IS_ADDR_LINKLOCAL(a) ((((const uint8_t *) (a))[0] == 0xfe) && \
                                      ((((const uint8_t *) (a))[1] & 0xc0) == 0x80))
#endif
#ifndef IN6_IS_ADDR_SITELOCAL
#    define IN6_IS_ADDR_SITELOCAL(a) ((((const uint8_t *) (a))[0] == 0xfe) && \
                                      ((((const uint8_t *) (a))[1] & 0xc0) == 0xc0))
#endif
#ifndef IN6_IS_ADDR_V4MAPPED
#    define IN6_IS_ADDR_V4MAPPED(a) (((const uint32_t *) (a))[0] == 0 && \
                                     ((const uint32_t *) (a))[1] == 0 && \
                                     ((const uint32_t *) (a))[2] == 0xffff0000)
#endif
#ifndef IN6_IS_ADDR_V4COMPAT
#    define IN6_IS_ADDR_V4COMPAT(a) (((const uint32_t *) (a))[0] == 0 && \
                                     ((const uint32_t *) (a))[1] == 0 && \
                                     ((const uint32_t *) (a))[2] == 0 && \
                                     ((const uint32_t *) (a))[3] != 0 && \
                                     ((const uint32_t *) (a))[3] != 0x01000000)
#endif
#ifndef IN6_ARE_ADDR_EQUAL
#    define IN6_ARE_ADDR_EQUAL(a, b) (0 == memcmp((a), (b), sizeof(struct in6_addr)))
#endif
#ifndef IN6_IS_ADDR_MC_LINKLOCAL
#    define IN6_IS_ADDR_MC_LINKLOCAL(a) (IN6_IS_ADDR_MULTICAST(a) && \
                                         ((((const uint8_t *) (a))[1] & 0xf) == 0x2))
#endif

#ifndef IPPORT_RESERVED
#    define IPPORT_RESERVED 1024
#endif
#ifndef IPPORT_USERRESERVED
#    define IPPORT_USERRESERVED 5000
#endif

/* byte order helpers live in winsock2.h (via common) */
#ifndef INADDR_LOOPBACK_HOST_ORDER
#    define INADDR_LOOPBACK_HOST_ORDER 0x0100007f
#endif

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_NETINET_IN_H */
