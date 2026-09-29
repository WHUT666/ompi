/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * <endian.h> for the native Windows build (x86/x64 are little-endian).
 */
#ifndef OPAL_WIN32_ENDIAN_H
#define OPAL_WIN32_ENDIAN_H

#ifndef __LITTLE_ENDIAN
#    define __LITTLE_ENDIAN 1234
#endif
#ifndef __BIG_ENDIAN
#    define __BIG_ENDIAN 4321
#endif
#ifndef __PDP_ENDIAN
#    define __PDP_ENDIAN 3412
#endif
#ifndef __BYTE_ORDER
#    define __BYTE_ORDER __LITTLE_ENDIAN
#endif

#ifndef LITTLE_ENDIAN
#    define LITTLE_ENDIAN __LITTLE_ENDIAN
#endif
#ifndef BIG_ENDIAN
#    define BIG_ENDIAN __BIG_ENDIAN
#endif
#ifndef PDP_ENDIAN
#    define PDP_ENDIAN __PDP_ENDIAN
#endif
#ifndef BYTE_ORDER
#    define BYTE_ORDER __BYTE_ORDER
#endif

#include <stdlib.h>

#define htobe16(x) _byteswap_ushort(x)
#define htole16(x) (x)
#define be16toh(x) _byteswap_ushort(x)
#define le16toh(x) (x)
#define htobe32(x) _byteswap_ulong(x)
#define htole32(x) (x)
#define be32toh(x) _byteswap_ulong(x)
#define le32toh(x) (x)
#define htobe64(x) _byteswap_uint64(x)
#define htole64(x) (x)
#define be64toh(x) _byteswap_uint64(x)
#define le64toh(x) (x)

#define htobe16f htobe16
#define betoh16  be16toh
#define betoh32  be32toh
#define betoh64  be64toh
#define letoh16  le16toh
#define letoh32  le32toh
#define letoh64  le64toh

#endif /* OPAL_WIN32_ENDIAN_H */
