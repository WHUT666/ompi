/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <pwd.h> replacement for the native Windows build.  Supplies
 * the current user's name/home from the environment.
 */
#ifndef OPAL_WIN32_PWD_H
#define OPAL_WIN32_PWD_H

#include "opal_win32_common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct passwd {
    char   *pw_name;
    char   *pw_passwd;
    uid_t   pw_uid;
    gid_t   pw_gid;
    char   *pw_gecos;
    char   *pw_dir;
    char   *pw_shell;
};

struct passwd *getpwuid(uid_t uid);
struct passwd *getpwnam(const char *name);
OPAL_WIN32_DECLSPEC int getpwuid_r(uid_t uid, struct passwd *pwd, char *buf,
                          size_t buflen, struct passwd **result);
OPAL_WIN32_DECLSPEC int getpwnam_r(const char *name, struct passwd *pwd, char *buf,
                          size_t buflen, struct passwd **result);
struct passwd *getpwent(void);
OPAL_WIN32_DECLSPEC void setpwent(void);
OPAL_WIN32_DECLSPEC void endpwent(void);

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_PWD_H */
