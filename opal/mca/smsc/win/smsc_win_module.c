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

OBJ_CLASS_INSTANCE(mca_smsc_win_endpoint_t, opal_object_t, NULL, NULL);

mca_smsc_endpoint_t *mca_smsc_win_get_endpoint(opal_proc_t *peer_proc)
{
    mca_smsc_win_endpoint_t *endpoint = OBJ_NEW(mca_smsc_win_endpoint_t);
    if (OPAL_UNLIKELY(NULL == endpoint)) {
        return NULL;
    }

    endpoint->super.proc = peer_proc;

    int rc;
    size_t modex_size;
    mca_smsc_win_modex_t *modex;
    OPAL_MODEX_RECV_IMMEDIATE(rc, &mca_smsc_win_component.smsc_version, &peer_proc->proc_name,
                              (void **) &modex, &modex_size);
    if (OPAL_UNLIKELY(OPAL_SUCCESS != rc)) {
        OBJ_RELEASE(endpoint);
        return NULL;
    }

    DWORD peer_pid = modex->pid;
    free(modex);

    HANDLE handle = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE
                                    | PROCESS_QUERY_INFORMATION,
                                FALSE, peer_pid);
    if (NULL == handle) {
        opal_output_verbose(MCA_BASE_VERBOSE_ERROR, opal_smsc_base_framework.framework_output,
                            "mca_smsc_win_get_endpoint: OpenProcess(%lu) failed (winerr %lu); "
                            "cannot use single-copy with this peer",
                            (unsigned long) peer_pid, (unsigned long) GetLastError());
        OBJ_RELEASE(endpoint);
        return NULL;
    }

    endpoint->proc_handle = handle;
    return &endpoint->super;
}

void mca_smsc_win_return_endpoint(mca_smsc_endpoint_t *endpoint)
{
    mca_smsc_win_endpoint_t *win_endpoint = (mca_smsc_win_endpoint_t *) endpoint;
    if (NULL != win_endpoint->proc_handle) {
        CloseHandle(win_endpoint->proc_handle);
        win_endpoint->proc_handle = NULL;
    }
    OBJ_RELEASE(endpoint);
}

static int mca_smsc_win_copy(mca_smsc_endpoint_t *endpoint, void *local_address,
                             void *remote_address, size_t size, BOOL write)
{
    mca_smsc_win_endpoint_t *win_endpoint = (mca_smsc_win_endpoint_t *) endpoint;
    SIZE_T done = 0;
    SIZE_T total = 0;

    while (total < size) {
        SIZE_T chunk = size - total;
        BOOL ok;
        if (write) {
            ok = WriteProcessMemory(win_endpoint->proc_handle,
                                    (LPVOID) ((uintptr_t) remote_address + total),
                                    (LPCVOID) ((uintptr_t) local_address + total), chunk, &done);
        } else {
            ok = ReadProcessMemory(win_endpoint->proc_handle,
                                   (LPCVOID) ((uintptr_t) remote_address + total),
                                   (LPVOID) ((uintptr_t) local_address + total), chunk, &done);
        }
        if (!ok || 0 == done) {
            OPAL_OUTPUT_VERBOSE((MCA_BASE_VERBOSE_ERROR,
                                 opal_smsc_base_framework.framework_output,
                                 "smsc_win %s %zu of %zu bytes, winerr = %lu",
                                 write ? "wrote" : "read", (size_t) total, size,
                                 (unsigned long) GetLastError()));
            return OPAL_ERROR;
        }
        total += done;
    }

    return OPAL_SUCCESS;
}

int mca_smsc_win_copy_to(mca_smsc_endpoint_t *endpoint, void *local_address, void *remote_address,
                         size_t size, void *reg_handle)
{
    /* ignore the registration handle as it is not used for win */
    (void) reg_handle;
    return mca_smsc_win_copy(endpoint, local_address, remote_address, size, TRUE);
}

int mca_smsc_win_copy_from(mca_smsc_endpoint_t *endpoint, void *local_address, void *remote_address,
                           size_t size, void *reg_handle)
{
    /* ignore the registration handle as it is not used for win */
    (void) reg_handle;
    return mca_smsc_win_copy(endpoint, local_address, remote_address, size, FALSE);
}

/* unsupported interfaces defined to support MCA direct */
void *mca_smsc_win_map_peer_region(mca_smsc_endpoint_t *endpoint, uint64_t flags,
                                   void *remote_address, size_t size, void **local_mapping)
{
    return NULL;
}

void mca_smsc_win_unmap_peer_region(void *ctx)
{
}

void *mca_smsc_win_register_region(void *local_address, size_t size)
{
    return NULL;
}

void mca_smsc_win_deregister_region(void *reg_data)
{
}

mca_smsc_module_t mca_smsc_win_module = {
    .get_endpoint = mca_smsc_win_get_endpoint,
    .return_endpoint = mca_smsc_win_return_endpoint,
    .copy_to = mca_smsc_win_copy_to,
    .copy_from = mca_smsc_win_copy_from,
};
