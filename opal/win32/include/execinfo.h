/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * <execinfo.h> replacement for the native Windows build, backed by
 * CaptureStackBackTrace().
 */
#ifndef OPAL_WIN32_EXECINFO_H
#define OPAL_WIN32_EXECINFO_H

#ifdef __cplusplus
extern "C" {
#endif

OPAL_WIN32_DECLSPEC int backtrace(void **buffer, int size);
char **backtrace_symbols(void *const *buffer, int size);
OPAL_WIN32_DECLSPEC void backtrace_symbols_fd(void *const *buffer, int size, int fd);

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_EXECINFO_H */
