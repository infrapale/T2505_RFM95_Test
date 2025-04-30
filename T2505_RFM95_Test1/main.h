#ifndef __MAIN_H__
#define __MAIN_H__

#define LABEL_LEN           12
#define TIME_ZONE_OFFS      3
#define UNIT_LABEL_LEN      10
#define MEASURE_LABEL_LEN   16

#define APP_NAME   "T2504 Pico RFM95 80x70"
#define MAIN_TITLE "RFM95 Test"

typedef enum
{
    NODE_CLIENT = 0,
    NODE_SERVER = 1
} node_type_et;

typedef struct 
{
    node_type_et    node_type; 
} node_ctrl_st;

typedef struct date_time
{
    uint16_t  year;
    uint8_t   month;
    uint8_t   day;
    uint8_t   hour;
    uint8_t   minute;
    uint8_t   second;
 } date_time_st;

#endif