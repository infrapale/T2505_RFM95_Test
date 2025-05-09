#include "main.h"
#include "uart.h"
#include "json.h"
#include "rfm_receive.h"
#include "rfm_send.h"
#include "io.h"

#define Uart Serial

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


uart_msg_st         uart;

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
  String  str;
  field_st  field;
  uint8_t rx_delim[field_nbr_of];
} rx_st;



uart_msg_st *uart_get_data_ptr(void)
{
    return &uart;
}

void uart_initialize(void)
{
    uart.rx.avail = false;
}

void uart_read_uart(void)
{
    if (SerialX.available())
    {
        io_led_flash(LED_INDX_BLUE,20);
        uart.rx.str = SerialX.readStringUntil('\n');
        if (uart.rx.str.length()> 0)
        {
            uart.rx.avail = true;
            //uart.rx.str.remove(uart.rx.str.length()-1);
        }
        #ifdef DEBUG_PRINT
        Serial.println("rx is available");
        #endif        
    } 

}

void uart_report_radio_msg(char *radio_receive_msg, int rssi)
{
    String msg_str = radio_receive_msg;
    Serial.print(msg_str); Serial.print(" - ");
    Serial.println(rssi);
}

void uart_parse_rx_frame(void)
{
    //rfm_send_msg_st *rx_msg = &send_msg; 
    bool do_continue = true;
    uint8_t len;
    uart.rx.str.trim();
    uart.rx.len = uart.rx.str.length();
    if ((uart.rx.str.charAt(0) != '<') || 
        (uart.rx.str.charAt(uart.rx.len-1) != '>'))  do_continue = false;
    if (do_continue)
    {   
        uart.rx.status = STATUS_OK_FOR_ME;
        uart.rx.avail = true;
        #ifdef DEBUG_PRINT
        Serial.print("Buffer frame is OK\n");
        #endif
    }
    else uart.rx.status = STATUS_INCORRECT_FRAME;
}

void uart_build_node_from_rx_str(void)
{
    uint8_t indx1;
    uint8_t indx2;
    indx1 = 0;  //uart.rx.str.indexOf(':')
    indx2 = uart.rx.str.indexOf(';');
    uart.node.zone = uart.rx.str.substring(indx1,indx2);
    indx1 = indx2+1;
    indx2 = uart.rx.str.indexOf(';',indx1+1);
    uart.node.name = uart.rx.str.substring(indx1,indx2);
    indx1 = indx2+1;
    indx2 = uart.rx.str.indexOf(';',indx1+1);
    uart.node.value = uart.rx.str.substring(indx1,indx2);
    indx1 = indx2+1;
    indx2 = uart.rx.str.indexOf(';',indx1+1);
    uart.node.remark = uart.rx.str.substring(indx1,indx2);
    indx1 = indx2+1;
    indx2 = uart.rx.str.indexOf(';',indx1+1);
    
}







void uart_build_raw_tx_str(void)
{
    rfm_receive_msg_st *receive_p = rfm_receive_get_data_ptr();
    uart.tx.str += (char*) receive_p->radio_msg;
}

void uart_rx_send_rfm_from_raw(void)
{
    // String payload = uart.rx.str.substring(6,uart.rx.len - 1);
    String payload = uart.rx.str;
    payload.toCharArray(uart.rx.radio_msg, MAX_MESSAGE_LEN);
    rfm_send_radiate_msg(uart.rx.radio_msg);
}

void uart_rx_send_rfm_from_node(void)
{
    uart.rx.str = uart.rx.str.substring(6,uart.rx.len - 1);
    uart_build_node_from_rx_str();
    rfm_send_msg_st *send_p = rfm_send_get_data_ptr();
    json_convert_uart_node_to_json(send_p->radio_msg, &uart);
    rfm_send_radiate_msg(send_p->radio_msg);
}

void uart_exec_cmnd(uart_cmd_et ucmd)
{
    uart_rx_send_rfm_from_raw();
}


void uart_print_rx_metadata(void)
{
    Serial.print("Length      "); Serial.println(uart.rx.len);
    Serial.print("Avail       "); Serial.println(uart.rx.avail);
    Serial.print("Status      "); Serial.println(uart.rx.status);
    Serial.print("Module      "); Serial.println(uart.rx.module);
    Serial.print("Address     "); Serial.println(uart.rx.addr);
    Serial.print("Command     "); Serial.println(uart.rx.cmd);
    Serial.print("Format      "); Serial.println(uart.rx.format);
}    


