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
#include "opal_config.h"

#include "opal/mca/pmix/pmix-internal.h"
#include "opal/mca/smsc/base/base.h"
#include "opal/mca/smsc/win/smsc_win_internal.h"

static int mca_smsc_win_component_register(void);
static int mca_smsc_win_component_open(void);
static int mca_smsc_win_component_close(void);
static int mca_smsc_win_component_query(void);
static mca_smsc_module_t *mca_smsc_win_component_enable(void);

#define MCA_SMSC_WIN_DEFAULT_PRIORITY 37
static const int mca_smsc_win_default_priority = MCA_SMSC_WIN_DEFAULT_PRIORITY;

mca_smsc_component_t mca_smsc_win_component = {
    .smsc_version = {
        MCA_SMSC_DEFAULT_VERSION("win"),
        .mca_open_component = mca_smsc_win_component_open,
        .mca_close_component = mca_smsc_win_component_close,
        .mca_register_component_params = mca_smsc_win_component_register,
    },
    .priority = MCA_SMSC_WIN_DEFAULT_PRIORITY,
    .query = mca_smsc_win_component_query,
    .enable = mca_smsc_win_component_enable,
};
MCA_BASE_COMPONENT_INIT(opal, smsc, win)

static int mca_smsc_win_component_register(void)
{
    mca_smsc_base_register_default_params(&mca_smsc_win_component, mca_smsc_win_default_priority);
    return OPAL_SUCCESS;
}

static int mca_smsc_win_component_open(void)
{
    /* nothing to do */
    return OPAL_SUCCESS;
}

static int mca_smsc_win_component_close(void)
{
    /* nothing to do */
    return OPAL_SUCCESS;
}

static int mca_smsc_win_component_query(void)
{
    mca_smsc_win_modex_t modex;
    int rc;

    /* ReadProcessMemory/WriteProcessMemory are always available between
     * same-user processes on Windows; there is no ptrace-scope-style
     * knob to probe.  Access checks happen per-peer in get_endpoint. */
    modex.pid = GetCurrentProcessId();
    OPAL_MODEX_SEND(rc, PMIX_LOCAL, &mca_smsc_win_component.smsc_version, &modex, sizeof(modex));
    return rc;
}

static mca_smsc_module_t *mca_smsc_win_component_enable(void)
{
    if (0 > mca_smsc_win_component.priority) {
        return NULL;
    }

    return &mca_smsc_win_module;
}
