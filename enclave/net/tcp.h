#ifndef TCP_H
#define TCP_H
#include "../include/config.h"

typedef uint32_t tcp_seq_t;
struct click_tcp {
    uint16_t    th_sport;       /* 0-1   source port             */
    uint16_t    th_dport;       /* 2-3   destination port        */
    tcp_seq_t   th_seq;         /* 4-7   sequence number         */
    tcp_seq_t   th_ack;         /* 8-11  acknowledgement number      */
#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
    unsigned    th_flags2 : 4;      /* 12    more flags          */
    unsigned    th_off : 4;     /*       data offset in words        */
#elif CLICK_BYTE_ORDER == CLICK_BIG_ENDIAN
    unsigned    th_off : 4;     /* 12    data offset in words        */
    unsigned    th_flags2 : 4;      /*       more flags          */
#else
#   error "unknown byte order"
#endif
#define TH_NS       0x01        /*       in 'th_flags2'          */
    uint8_t th_flags;       /* 13    flags               */
#define TH_FIN      0x01
#define TH_SYN      0x02
#define TH_RST      0x04
#define TH_PUSH     0x08
#define TH_ACK      0x10
#define TH_URG      0x20
#define TH_ECE      0x40
#define TH_CWR      0x80
    uint16_t    th_win;         /* 14-15 window              */
    uint16_t    th_sum;         /* 16-17 checksum            */
    uint16_t    th_urp;         /* 18-19 urgent pointer          */
};
#endif