/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <dlfcn.h> replacement for the native Windows build.
 */
#ifndef OPAL_WIN32_DLFCN_H
#define OPAL_WIN32_DLFCN_H

#include "opal_win32_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RTLD_LAZY      0x0001
#define RTLD_NOW       0x0002
#define RTLD_GLOBAL    0x0100
#define RTLD_LOCAL     0x0200
#define RTLD_NODELETE  0x0400
#define RTLD_NOLOAD    0x0800
#define RTLD_DEEPBIND  0x1000
#define RTLD_DEFAULT   ((void *) 0)
#define RTLD_NEXT      ((void *) -1)

OPAL_WIN32_DECLSPEC void *opal_win32_dlopen(const char *filename, int flag);
OPAL_WIN32_DECLSPEC char *opal_win32_dlerror(void);
OPAL_WIN32_DECLSPEC void *opal_win32_dlsym(void *handle, const char *symbol);
OPAL_WIN32_DECLSPEC int   opal_win32_dlclose(void *handle);
OPAL_WIN32_DECLSPEC int   opal_win32_dladdr(const void *addr, void *info);

#define dlopen  opal_win32_dlopen
#define dlerror opal_win32_dlerror
#define dlsym   opal_win32_dlsym
#define dlclose opal_win32_dlclose
#define dladdr  opal_win32_dladdr

typedef struct {
    const char *dli_fname;
    void       *dli_fbase;
    const char *dli_sname;
    void       *dli_saddr;
} Dl_info;

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_DLFCN_H */
