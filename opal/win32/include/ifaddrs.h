/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * <ifaddrs.h> replacement for the native Windows build -- backed by
 * GetAdaptersAddresses() (see opal/win32/opal_win32_net.c).
 */
#ifndef OPAL_WIN32_IFADDRS_H
#define OPAL_WIN32_IFADDRS_H

#include "opal_win32_common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ifaddrs {
    struct ifaddrs  *ifa_next;
    char            *ifa_name;
    unsigned int     ifa_flags;
    struct sockaddr *ifa_addr;
    struct sockaddr *ifa_netmask;
    union {
        struct sockaddr *ifu_broadaddr;
        struct sockaddr *ifu_dstaddr;
    } ifa_ifu;
    void *ifa_data;
};

#define ifa_broadaddr ifa_ifu.ifu_broadaddr
#define ifa_dstaddr   ifa_ifu.ifu_dstaddr

OPAL_WIN32_DECLSPEC int  opal_win32_getifaddrs(struct ifaddrs **ifap);
OPAL_WIN32_DECLSPEC void opal_win32_freeifaddrs(struct ifaddrs *ifa);

#define getifaddrs  opal_win32_getifaddrs
#define freeifaddrs opal_win32_freeifaddrs

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_IFADDRS_H */
