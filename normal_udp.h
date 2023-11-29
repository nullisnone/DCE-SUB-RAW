#ifndef NORMAL_UDP_H
#define NORMAL_UDP_H

#include <stdint.h>
#include <stdio.h>

#pragma pack(1)

typedef struct  {
        char 	 contract_id[8];
        double	 last_price;
        uint32_t last_match_qty;
        uint32_t match_tot_qty;
        uint32_t open_interest;
        double   bid_price;
        uint32_t bid_qty;
        double   ask_price;
        uint32_t ask_qty;
} normal_best_quote;

typedef struct  {
        char 	 contract_id[8];
        double   bid1_price;
        uint32_t bid1_qty;
        double   bid2_price;
        uint32_t bid2_qty;
        double   bid3_price;
        uint32_t bid3_qty;
        double   bid4_price;
        uint32_t bid4_qty;
        double   bid5_price;
        uint32_t bid5_qty;
        double   ask1_price;
        uint32_t ask1_qty;
        double   ask2_price;
        uint32_t ask2_qty;
        double   ask3_price;
        uint32_t ask3_qty;
        double   ask4_price;
        uint32_t ask4_qty;
        double   ask5_price;
        uint32_t ask5_qty;
} normal_depth_update;

#pragma pack()

#endif
