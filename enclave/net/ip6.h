#ifndef IP6_H
#define IP6_H
#include "../include/config.h"

struct in6_addr{
    uint8_t s6_addr[16]; /* IPv6 address */
};


struct click_ip6 {
    union {
    struct {
        uint32_t ip6_un1_flow;  /* 0-3   bits 0-3: version == 6      */
                    /*   bits 4-11: traffic class    */
                    /*     bits 4-9: DSCP        */
                    /*     bits 10-11: ECN       */
                    /*   bits 12-31: flow label      */
        uint16_t ip6_un1_plen;  /* 4-5   payload length          */
        uint8_t ip6_un1_nxt;    /* 6     next header             */
        uint8_t ip6_un1_hlim;   /* 7     hop limit           */
    } ip6_un1;
    uint8_t ip6_un2_vfc;        /* 0     bits 0-3: version == 6      */
                    /*   bits 4-7: top 4 class bits  */
    struct {
#if CLICK_BYTE_ORDER == CLICK_BIG_ENDIAN
        unsigned ip6_un3_v : 4; /* 0     version == 6            */
        unsigned ip6_un3_fc : 4;    /*   header length           */
#elif CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
        unsigned ip6_un3_fc : 4;    /* 0     header length           */
        unsigned ip6_un3_v : 4; /*   version == 6            */
#endif
    } ip6_un3;
    } ip6_ctlun;
    struct in6_addr ip6_src;    /* 8-23  source address */
    struct in6_addr ip6_dst;    /* 24-39 dest address */
};
#endif
