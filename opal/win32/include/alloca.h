/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <alloca.h> replacement for the native Windows build.
 */
#ifndef OPAL_WIN32_ALLOCA_H
#define OPAL_WIN32_ALLOCA_H

#include "opal_win32_common.h"
#include <malloc.h>

#ifndef alloca
#    define alloca _alloca
#endif

#endif /* OPAL_WIN32_ALLOCA_H */
