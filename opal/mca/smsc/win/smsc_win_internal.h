/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 * SPDX-License-Identifier: BSD-3-Clause-Open-MPI
 */

#ifndef OPAL_MCA_SMSC_WIN_SMSC_WIN_INTERNAL_H
#define OPAL_MCA_SMSC_WIN_SMSC_WIN_INTERNAL_H

#include "opal/mca/smsc/win/smsc_win.h"
#include "opal_win32_common.h"

/* Modex payload: the peer's Win32 process id.  No registration data
 * is needed -- ReadProcessMemory/WriteProcessMemory can address any
 * readable/writable region of a peer we were able to OpenProcess(). */
struct mca_smsc_win_modex_t {
    DWORD pid;
};

typedef struct mca_smsc_win_modex_t mca_smsc_win_modex_t;

struct mca_smsc_win_endpoint_t {
    mca_smsc_endpoint_t super;
    /** handle for the peer process (VM read/write/operate). */
    HANDLE proc_handle;
};

typedef struct mca_smsc_win_endpoint_t mca_smsc_win_endpoint_t;

OBJ_CLASS_DECLARATION(mca_smsc_win_endpoint_t);

#endif /* OPAL_MCA_SMSC_WIN_SMSC_WIN_INTERNAL_H */
