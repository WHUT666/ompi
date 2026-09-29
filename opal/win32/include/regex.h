/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 * SPDX-License-Identifier: BSD-3-Clause-Open-MPI
 *
 * Minimal POSIX regex shim for the Windows port.  The CRT has no
 * regex support; only a stub surface is provided (the sole consumer,
 * opal/mca/pmix, only compiles the include today).  Implementations
 * return REG_NOSYS so any future caller fails loudly rather than
 * silently matching nothing.
 */
#ifndef OPAL_WIN32_REGEX_H
#define OPAL_WIN32_REGEX_H

#include "opal_win32_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef OPAL_WIN32_REGOFF_DEFINED
#    define OPAL_WIN32_REGOFF_DEFINED
typedef long regoff_t;
#endif

typedef struct {
    void *re_nsub_opaque;
    size_t re_nsub;
    int cflags;
} regex_t;

typedef struct {
    regoff_t rm_so;
    regoff_t rm_eo;
} regmatch_t;

/* regcomp flags */
#define REG_EXTENDED  0x0001
#define REG_ICASE     0x0002
#define REG_NOSUB     0x0004
#define REG_NEWLINE   0x0008
/* regexec flags */
#define REG_NOTBOL    0x0100
#define REG_NOTEOL    0x0200
/* error codes */
#define REG_OK        0
#define REG_NOMATCH   1
#define REG_BADPAT    2
#define REG_ECOLLATE  3
#define REG_ECTYPE    4
#define REG_EESCAPE   5
#define REG_ESUBREG   6
#define REG_EBRACK    7
#define REG_EPAREN    8
#define REG_EBRACE    9
#define REG_BADBR     10
#define REG_ERANGE    11
#define REG_ESPACE    12
#define REG_BADRPT    13
#define REG_NOSYS     14

OPAL_WIN32_DECLSPEC int     opal_win32_regcomp(regex_t *preg, const char *pattern, int cflags);
OPAL_WIN32_DECLSPEC int     opal_win32_regexec(const regex_t *preg, const char *string,
                           size_t nmatch, regmatch_t pmatch[], int eflags);
OPAL_WIN32_DECLSPEC size_t  opal_win32_regerror(int errcode, const regex_t *preg, char *errbuf,
                            size_t errbuf_size);
OPAL_WIN32_DECLSPEC void    opal_win32_regfree(regex_t *preg);

#define regcomp  opal_win32_regcomp
#define regexec  opal_win32_regexec
#define regerror opal_win32_regerror
#define regfree  opal_win32_regfree

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_REGEX_H */
