/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <libgen.h> replacement for the native Windows build.
 */
#ifndef OPAL_WIN32_LIBGEN_H
#define OPAL_WIN32_LIBGEN_H

#include "opal_win32_common.h"

#ifdef __cplusplus
extern "C" {
#endif

OPAL_WIN32_DECLSPEC char *opal_win32_basename(char *path);
OPAL_WIN32_DECLSPEC char *opal_win32_dirname(char *path);

#define basename opal_win32_basename
#define dirname  opal_win32_dirname

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_LIBGEN_H */
