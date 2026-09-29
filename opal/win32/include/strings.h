/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <strings.h> replacement for the native Windows build.
 */
#ifndef OPAL_WIN32_STRINGS_H
#define OPAL_WIN32_STRINGS_H

#include "opal_win32_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define strcasecmp  _stricmp
#define strncasecmp _strnicmp
#define index       strchr
#define rindex      strrchr

int     ffs(int value);
int     fls(int value);
int     flsl(long value);
char   *strsep(char **stringp, const char *delim);
int     strcasecmp_l(const char *s1, const char *s2, void *loc);
int     strncasecmp_l(const char *s1, const char *s2, size_t n, void *loc);
int     bcmp(const void *s1, const void *s2, size_t n);
void    bcopy(const void *src, void *dest, size_t n);
static inline void bzero(void *s, size_t n)
{
    memset(s, 0, n);
}

static inline int opal_win32_ffs(int value)
{
    DWORD idx;
    if (0 == value) {
        return 0;
    }
    _BitScanForward(&idx, (DWORD) value);
    return (int) idx + 1;
}
#define ffs opal_win32_ffs

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_STRINGS_H */
