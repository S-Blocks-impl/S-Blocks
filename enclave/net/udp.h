#ifndef UDP_H
#define UDP_H

#include "../include/config.h"

struct click_udp {
    uint16_t    uh_sport;       /* 0-1   source port             */
    uint16_t    uh_dport;       /* 2-3   destination port        */
    uint16_t    uh_ulen;        /* 4-5   UDP length          */
    uint16_t    uh_sum;         /* 6-7   checksum            */
};
#endif