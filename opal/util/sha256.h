/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2022      Triad National Security, LLC. All rights
 *                         reserved.
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 * SPDX-License-Identifier: BSD-3-Clause-Open-MPI
 */

/*********************************************************************
* Filename:   sha256.h
* Author:     Brad Conte (brad AT bradconte.com)
* Copyright:
* Disclaimer: This code is presented "as is" without any guarantees.
* Details:    Defines the API for the corresponding SHA1 implementation.
* Notes:      see https://github.com/B-Con/crypto-algorithms
*********************************************************************/

#ifndef OPAL_SHA256_H
#define OPAL_SHA256_H

/*************************** HEADER FILES ***************************/
#include <stddef.h>
#include <stdint.h>

/****************************** MACROS ******************************/
#define OPAL_SHA256_BLOCK_SIZE 32            // SHA256 outputs a 32 byte digest

/**************************** DATA TYPES ****************************/
/* NOTE: fixed-width prefixed names -- the original code typedef'd
 * BYTE/WORD which collide with the Win32 SDK types on Windows */
typedef uint8_t  opal_sha256_byte_t;
typedef uint32_t opal_sha256_word_t;

typedef struct {
    opal_sha256_byte_t data[64];
    opal_sha256_word_t datalen;
    unsigned long long bitlen;
    opal_sha256_word_t state[8];
} opal_sha256_ctx;

/*********************** FUNCTION DECLARATIONS **********************/
void opal_sha256_init(opal_sha256_ctx *ctx);
void opal_sha256_update(opal_sha256_ctx *ctx, const opal_sha256_byte_t data[], size_t len);
void opal_sha256_final(opal_sha256_ctx *ctx, opal_sha256_byte_t hash[]);

#endif   // OPAL_SHA256_H
