/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <grp.h> stub for the native Windows build.
 */
#ifndef OPAL_WIN32_GRP_H
#define OPAL_WIN32_GRP_H

#include "opal_win32_common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct group {
    char   *gr_name;
    char   *gr_passwd;
    gid_t   gr_gid;
    char  **gr_mem;
};

struct group *getgrgid(gid_t gid);
struct group *getgrnam(const char *name);
OPAL_WIN32_DECLSPEC int getgrgid_r(gid_t gid, struct group *grp, char *buf,
                         size_t buflen, struct group **result);
OPAL_WIN32_DECLSPEC int getgrnam_r(const char *name, struct group *grp, char *buf,
                         size_t buflen, struct group **result);
struct group *getgrent(void);
OPAL_WIN32_DECLSPEC void setgrent(void);
OPAL_WIN32_DECLSPEC void endgrent(void);
OPAL_WIN32_DECLSPEC int getgrouplist(const char *user, gid_t group, gid_t *groups,
                           int *ngroups);
OPAL_WIN32_DECLSPEC int setgroups(int ngroups, const gid_t *groups);

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_GRP_H */
