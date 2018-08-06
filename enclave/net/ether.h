#ifndef ETHER_H
#define ETHER_H

#include "../include/config.h"

struct click_ether {
    uint8_t ether_dhost[6];     /* 0-5   Ethernet destination address */
    uint8_t ether_shost[6];     /* 6-11  Ethernet source address      */
    uint16_t    ether_type;     /* 12-13 Ethernet protocol        */
} CLICK_SIZE_PACKED_ATTRIBUTE;
#endif