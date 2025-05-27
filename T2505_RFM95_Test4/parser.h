#ifndef __PARSER_H__
#define __PARSER_H__


typedef enum
{
  FIELD_START   = 0,
  FIELD_COMMAND,
  FIELD_FROM,
  FIELD_TARGET,
  FIELD_RADIO,
  FIELD_POWER,
  FIELD_SPREADING_FACTOR,
  FIELD_REMOTE_NBR,
  FIELD_BASE_NBR,
  FILED_END,
  FIELD_NBR_OF
} field_et;

typedef enum
{
  RADIO_UNDEFINED = 0,
  RADIO_LORA_433  = 1,
  RADIO_LORA_868  = 2,
  RADIO_RFM69_433 = 3,
  RADIO_RFM69_868 = 4,
} radio_et;

typedef enum
{
  RX_MSG_SEND_REMOTE_TO_BASE,
  RX_MSG_RESET_CNTR,
  RX_MSG_,
  RX_MSG_NBR_OF
} rx_msg_et;

typedef enum
{
  TX_MSG_SEND_REMOTE_TO_BASE,
  TX_MSG_RESET_CNTR,
  TX_MSG_,
  TX_MSG_NBR_OF
} tx_msg_et;

typedef enum
{
  STATUS_UNDEFINED = 0,
  STATUS_OK_FOR_ME,
  STATUS_INCORRECT_FRAME,
} msg_status_et;


typedef struct
{
  char      start;
  char      cmnd[5];
  uint8_t   from;
  uint8_t   target;
  uint8_t   radio;
  uint8_t   power;
  uint8_t   sf;
  uint16_t  remote_nbr;
  uint16_t  base_nbr;
  char      end;
} field_st;



typedef struct
{
  String          str;
  uint8_t         len;
  field_st        field;
  msg_status_et   status;
  uint8_t         rx_delim[FIELD_NBR_OF];
  bool            avail;
} msg_st;

typedef struct
{
    uint8_t tindx;
    //msg_st rx;
    //msg_st tx;
} parser_ctrl_st;

void parser_initialize(void);


#endif
