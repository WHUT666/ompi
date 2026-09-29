/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <net/if.h> replacement for the native Windows build.
 */
#ifndef OPAL_WIN32_NET_IF_H
#define OPAL_WIN32_NET_IF_H

#include "opal_win32_common.h"
#include <netioapi.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef IFNAMSIZ
#    define IFNAMSIZ 44
#endif
#ifndef IF_NAMESIZE
#    define IF_NAMESIZE IFNAMSIZ
#endif
#ifndef IF_NAME_SIZE
#    define IF_NAME_SIZE IF_NAMESIZE
#endif

/* interface flags - translated by our getifaddrs implementation.
 * IMPORTANT: the values for names shared with the Windows SDK
 * (ws2ipdef.h: IFF_UP, IFF_BROADCAST, IFF_LOOPBACK, IFF_POINTTOPOINT,
 * IFF_MULTICAST) must match the SDK values exactly.  ws2ipdef defines
 * them unconditionally, so depending on include order either header may
 * win - identical values keep the produced/consumed bit sets consistent
 * regardless.  POSIX-only names occupy bits the SDK does not use. */
#ifndef IFF_UP
#    define IFF_UP            0x1
#endif
#ifndef IFF_BROADCAST
#    define IFF_BROADCAST     0x2
#endif
#ifndef IFF_LOOPBACK
#    define IFF_LOOPBACK      0x4
#endif
#ifndef IFF_POINTTOPOINT
#    define IFF_POINTTOPOINT  0x8
#endif
#ifndef IFF_POINTOPOINT
#    define IFF_POINTOPOINT   IFF_POINTTOPOINT
#endif
#ifndef IFF_MULTICAST
#    define IFF_MULTICAST     0x10
#endif
#ifndef IFF_NOTRAILERS
#    define IFF_NOTRAILERS    0x20
#endif
#ifndef IFF_RUNNING
#    define IFF_RUNNING       0x40
#endif
#ifndef IFF_NOARP
#    define IFF_NOARP         0x80
#endif
#ifndef IFF_PROMISC
#    define IFF_PROMISC       0x100
#endif
#ifndef IFF_ALLMULTI
#    define IFF_ALLMULTI      0x200
#endif
#ifndef IFF_DEBUG
#    define IFF_DEBUG         0x400
#endif

struct ifreq {
    char ifr_name[IFNAMSIZ];
    union {
        struct sockaddr ifr_addr;
        struct sockaddr ifr_dstaddr;
        struct sockaddr ifr_broadaddr;
        short           ifr_flags;
        int             ifr_metric;
        int             ifr_mtu;
        int             ifr_ifindex;
        void           *ifr_data;
    } ifr_ifru;
};
#define ifr_addr      ifr_ifru.ifr_addr
#define ifr_dstaddr   ifr_ifru.ifr_dstaddr
#define ifr_broadaddr ifr_ifru.ifr_broadaddr
#define ifr_flags     ifr_ifru.ifr_flags
#define ifr_metric    ifr_ifru.ifr_metric
#define ifr_mtu       ifr_ifru.ifr_mtu
#define ifr_ifindex   ifr_ifru.ifr_ifindex
#define ifr_data      ifr_ifru.ifr_data

struct ifconf {
    int ifc_len;
    union {
        void         *ifcu_buf;
        struct ifreq *ifcu_req;
    } ifc_ifcu;
};
#define ifc_buf ifc_ifcu.ifcu_buf
#define ifc_req ifc_ifcu.ifcu_req

/* POSIX-style if_nameindex: the Windows netioapi if_nameindex() returns
 * a different struct, so we provide our own.  We also wrap
 * if_nametoindex()/if_indextoname(): the netioapi versions do not
 * resolve the GUID-style AdapterName strings our getifaddrs() reports
 * in ifa_name, while our shims accept both AdapterName and
 * FriendlyName. */
struct if_nameindex {
    unsigned int if_index;
    char        *if_name;
};

OPAL_WIN32_DECLSPEC struct if_nameindex *opal_win32_if_nameindex(void);
OPAL_WIN32_DECLSPEC void                 opal_win32_if_freenameindex(struct if_nameindex *ptr);
OPAL_WIN32_DECLSPEC unsigned int         opal_win32_if_nametoindex(const char *ifname);
OPAL_WIN32_DECLSPEC char                *opal_win32_if_indextoname(unsigned int ifindex,
                                                                   char *ifname);

/* function-like macro so that "struct if_nameindex" is left alone */
#define if_nameindex() opal_win32_if_nameindex()
#define if_freenameindex(p) opal_win32_if_freenameindex((p))
#define if_nametoindex(p) opal_win32_if_nametoindex(p)
#define if_indextoname(i, p) opal_win32_if_indextoname((i), (p))

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_NET_IF_H */
