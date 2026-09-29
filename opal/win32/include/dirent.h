/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <dirent.h> replacement for the native Windows build,
 * backed by FindFirstFile()/FindNextFile().
 */
#ifndef OPAL_WIN32_DIRENT_H
#define OPAL_WIN32_DIRENT_H

#include "opal_win32_common.h"
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NAME_MAX
#    define NAME_MAX 255
#endif

typedef struct {
    long            d_ino;    /* always 0 on Windows */
    unsigned short  d_reclen;
    unsigned char   d_type;
    unsigned short  d_namlen;
    char            d_name[NAME_MAX + 1];
} dirent_t;

struct dirent {
    long           d_ino;
    unsigned short d_reclen;
    unsigned char  d_type;
    char           d_name[NAME_MAX + 1];
};

#define DT_UNKNOWN  0
#define DT_FIFO     1
#define DT_CHR      2
#define DT_DIR      4
#define DT_BLK      6
#define DT_REG      8
#define DT_LNK      10
#define DT_SOCK     12
#define DT_WHT      14

typedef struct DIR DIR;

DIR           *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
OPAL_WIN32_DECLSPEC int closedir(DIR *dirp);
OPAL_WIN32_DECLSPEC void rewinddir(DIR *dirp);
OPAL_WIN32_DECLSPEC long telldir(DIR *dirp);
OPAL_WIN32_DECLSPEC void seekdir(DIR *dirp, long loc);
OPAL_WIN32_DECLSPEC int dirfd(DIR *dirp);
DIR           *fdopendir(int fd);
OPAL_WIN32_DECLSPEC int scandir(const char *dirp, struct dirent ***namelist,
                       int (*filter)(const struct dirent *),
                       int (*compar)(const struct dirent **, const struct dirent **));
OPAL_WIN32_DECLSPEC int alphasort(const struct dirent **a, const struct dirent **b);
OPAL_WIN32_DECLSPEC int versionsort(const struct dirent **a, const struct dirent **b);

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_DIRENT_H */
