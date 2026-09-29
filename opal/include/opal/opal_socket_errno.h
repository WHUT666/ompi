/*
 * Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2005 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 * SPDX-License-Identifier: BSD-3-Clause-Open-MPI
 */
#ifndef OPAL_GET_SOCKET_ERROR_H
#define OPAL_GET_SOCKET_ERROR_H

#include "opal/constants.h"
#include <errno.h>

#ifdef _WIN32
/* Winsock does not report errors through errno; opal_win32_socket_errno()
 * maps WSAGetLastError() onto the corresponding POSIX errno value. */
BEGIN_C_DECLS
OPAL_DECLSPEC int opal_win32_socket_errno(void);
OPAL_DECLSPEC int opal_win32_socket_startup(void);
END_C_DECLS
#    define opal_socket_errno opal_win32_socket_errno()
#else
#    define opal_socket_errno errno
#endif

#endif /* OPAL_GET_ERROR_H */
