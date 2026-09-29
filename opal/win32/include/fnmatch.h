/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <fnmatch.h> replacement for the native Windows build.
 */
#ifndef OPAL_WIN32_FNMATCH_H
#define OPAL_WIN32_FNMATCH_H

#ifdef __cplusplus
extern "C" {
#endif

#define FNM_NOMATCH   1
#define FNM_NOESCAPE  0x01
#define FNM_PATHNAME  0x02
#define FNM_PERIOD    0x04
#define FNM_FILE_NAME FNM_PATHNAME
#define FNM_LEADING_DIR 0x08
#define FNM_CASEFOLD  0x10
#define FNM_EXTMATCH  0x20

OPAL_WIN32_DECLSPEC int fnmatch(const char *pattern, const char *string, int flags);

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_FNMATCH_H */
