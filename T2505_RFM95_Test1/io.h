#ifndef __IO_H__ 
#define __IO_H__
#include <Arduino.h>
//#include "main.h"

#define BOARD_T2504_PICO_RFM95_80x70    1
#define BOARD_PROTO_PICO_RFM95          2
#define BOARD BOARD_T2504_PICO_RFM95_80x70 

#define PIN_WIRE_SDA        (12u)
#define PIN_WIRE_SCL        (13u)


#if BOARD == BOARD_T2504_PICO_RFM95_80x70
#define PIN_RFM_MOSI        (19u)
#define PIN_RFM_MISO        (16u)
#define PIN_RFM_SCK         (18u)
#define PIN_RFM_CS          (17u)
#define PIN_RFM_IRQ         (21u)
#define PIN_RFM_RESET       (20u)

#elif BOARD == BOARD_PROTO_PICO_RFM95
#define PIN_RFM_MOSI        (19u)
#define PIN_RFM_MISO        (16u)
#define PIN_RFM_SCK         (18u)
#define PIN_RFM_CS          (10u)
#define PIN_RFM_IRQ         (21u)
#define PIN_RFM_RESET       (11u)
#else
#error No board was defined
#endif

#define PIN_SW1		          (15u)
#define PIN_SW2		          (14u)
#define PIN_SW3		          (13u)
#define PIN_SW4             (12u)

#define PIN_LED_RED         (2u)
#define PIN_LED_GREEN       (3u)
#define PIN_LED_BLUE        (22u)

#define LED_PATTERN_NBR_OF  8

typedef enum
{
  COLOR_RED = 0,
  COLOR_GREEN,
  COLOR_BLUE,
  COLOR_NBR_OF
} color_et;


void io_initialize(void);

void io_blink(uint8_t color, uint8_t pindx);

void io_task(void);

#endif