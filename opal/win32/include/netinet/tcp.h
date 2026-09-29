/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <netinet/tcp.h> replacement for the native Windows build.
 */
#ifndef OPAL_WIN32_NETINET_TCP_H
#define OPAL_WIN32_NETINET_TCP_H

#include "opal_win32_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* TCP_NODELAY is defined by winsock.  The rest are defined where the
 * Windows TCP stack supports them and mapped to no-ops where it does
 * not (setsockopt will just fail). */
#ifndef TCP_NODELAY
#    define TCP_NODELAY 0x0001
#endif
#ifndef TCP_MAXSEG
#    define TCP_MAXSEG 0x0002
#endif
#ifndef TCP_NOPUSH
#    define TCP_NOPUSH 0x0004
#endif
#ifndef TCP_NOOPT
#    define TCP_NOOPT 0x0008
#endif
#ifndef TCP_KEEPALIVE
#    define TCP_KEEPALIVE 0x0003
#endif
#ifndef TCP_KEEPIDLE
#    define TCP_KEEPIDLE 0x0003
#endif
#ifndef TCP_KEEPINTVL
#    define TCP_KEEPINTVL 0x0011
#endif
#ifndef TCP_KEEPCNT
#    define TCP_KEEPCNT 0x0010
#endif
#ifndef TCP_CORK
#    define TCP_CORK 0x0007
#endif
#ifndef TCP_QUICKACK
#    define TCP_QUICKACK 0x000c
#endif
#ifndef TCP_DEFER_ACCEPT
#    define TCP_DEFER_ACCEPT 0x0009
#endif
#ifndef TCP_INFO
#    define TCP_INFO 0x000b
#endif
#ifndef TCP_WINDOW_CLAMP
#    define TCP_WINDOW_CLAMP 0x000a
#endif
#ifndef TCP_CONGESTION
#    define TCP_CONGESTION 0x0010
#endif
#ifndef TCP_SYNCNT
#    define TCP_SYNCNT 0x0007
#endif
#ifndef TCP_USER_TIMEOUT
#    define TCP_USER_TIMEOUT 0x0012
#endif
#ifndef TCP_FASTOPEN
#    define TCP_FASTOPEN 0x000f
#endif

struct tcp_info {
    uint8_t  tcpi_state;
    uint8_t  tcpi_ca_state;
    uint8_t  tcpi_retransmits;
    uint8_t  tcpi_probes;
    uint8_t  tcpi_backoff;
    uint8_t  tcpi_options;
    uint8_t  tcpi_snd_wscale : 4, tcpi_rcv_wscale : 4;
    uint32_t tcpi_rto;
    uint32_t tcpi_ato;
    uint32_t tcpi_snd_mss;
    uint32_t tcpi_rcv_mss;
    uint32_t tcpi_unacked;
    uint32_t tcpi_sacked;
    uint32_t tcpi_lost;
    uint32_t tcpi_retrans;
    uint32_t tcpi_fackets;
    uint32_t tcpi_last_data_sent;
    uint32_t tcpi_last_ack_sent;
    uint32_t tcpi_last_data_recv;
    uint32_t tcpi_last_ack_recv;
    uint32_t tcpi_pmtu;
    uint32_t tcpi_rcv_ssthresh;
    uint32_t tcpi_rtt;
    uint32_t tcpi_rttvar;
    uint32_t tcpi_snd_ssthresh;
    uint32_t tcpi_snd_cwnd;
    uint32_t tcpi_advmss;
    uint32_t tcpi_reordering;
};

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_NETINET_TCP_H */
