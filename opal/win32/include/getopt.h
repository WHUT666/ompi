/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <getopt.h> replacement for the native Windows build.
 */
#ifndef OPAL_WIN32_GETOPT_H
#define OPAL_WIN32_GETOPT_H

#include "opal_win32_common.h"

#ifdef __cplusplus
extern "C" {
#endif

OPAL_WIN32_DECLSPEC extern char *opal_optarg;
OPAL_WIN32_DECLSPEC extern int   opal_optind;
OPAL_WIN32_DECLSPEC extern int   opal_opterr;
OPAL_WIN32_DECLSPEC extern int   opal_optopt;

#define optarg opal_optarg
#define optind opal_optind
#define opterr opal_opterr
#define optopt opal_optopt

OPAL_WIN32_DECLSPEC int opal_getopt(int argc, char *const argv[], const char *optstring);
#define getopt opal_getopt

struct option {
    const char *name;
    int         has_arg;
    int        *flag;
    int         val;
};

#define no_argument       0
#define required_argument 1
#define optional_argument 2

OPAL_WIN32_DECLSPEC int opal_getopt_long(int argc, char *const argv[], const char *optstring,
                     const struct option *longopts, int *longindex);
OPAL_WIN32_DECLSPEC int opal_getopt_long_only(int argc, char *const argv[], const char *optstring,
                          const struct option *longopts, int *longindex);

#define getopt_long      opal_getopt_long
#define getopt_long_only opal_getopt_long_only

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_GETOPT_H */
