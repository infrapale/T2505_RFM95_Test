#include <Arduino.h>
#include <stdint.h> 
#include "main.h"
#include "parser.h"
#include "atask.h"

#define UARTX0 Serial

msg_st tx_msg;
msg_st rx_msg;
parser_ctrl_st parser_ctrl;

void parser_task(void);

//                                  123456789012345   ival  next  state  prev  cntr flag  call backup
atask_st parser_task_handle    =  {"UART 0 Task    ", 1000,    0,     0,  255,    0,  1,  parser_task };


void parser_initialize(void)
{
    parser_ctrl.tindx =  atask_add_new(&parser_task_handle);
     rx_msg.avail = false;
}

void read_uart(void)
{
    if (UARTX0.available())
    {
        Serial.println("rx is available");
         rx_msg.str = UARTX0.readStringUntil('\n');
        if (rx_msg.str.length()> 0)
        {
            rx_msg.avail = true;
            //rx_msg.str.remove(rx_msg.str.length()-1);
            Serial.println(rx_msg.str);
        }
    } 

}


void parse_rx_frame(void)
{
    //rfm_send_msg_st *rx_msg = &send_msg; 
    bool do_continue = true;
    uint8_t len;
    rx_msg.str.trim();
    rx_msg.len = rx_msg.str.length();
    if ((rx_msg.str.charAt(0) != '<') || 
        (rx_msg.str.charAt(rx_msg.len-1) != '>'))  do_continue = false;
    if (do_continue)
    {   
        rx_msg.status = STATUS_OK_FOR_ME;
        rx_msg.avail = true;
        #ifdef DEBUG_PRINT
        Serial.print("Buffer frame is OK\n");
        #endif
    }
    else rx_msg.status = STATUS_INCORRECT_FRAME;
}

void uart_build_node_from_rx_str(void)
{
    uint8_t indx1;
    uint8_t indx2;
    //<UR2B;from;target;radio;freq;pwr;sf;rnbr;bnbr>\n


    indx1 = 1;  //rx_msg.str.indexOf(':')
    indx2 = rx_msg.str.indexOf(';');
    rx_msg.str.substring(indx1,indx2).toCharArray( rx_msg.field.cmnd, CMD_TAG_LEN);
    indx1 = indx2+1;
    indx2 = rx_msg.str.indexOf(';',indx1+1);
    rx_msg.field.from = rx_msg.str.substring(indx1,indx2).toInt();
    indx1 = indx2+1;
    indx2 = rx_msg.str.indexOf(';',indx1+1);
    rx_msg.field.target = rx_msg.str.substring(indx1,indx2).toInt();
    indx1 = indx2+1;
    indx2 = rx_msg.str.indexOf(';',indx1+1);
    rx_msg.field.radio = rx_msg.str.substring(indx1,indx2).toInt();
    indx1 = indx2+1;
    indx2 = rx_msg.str.indexOf(';',indx1+1);
    rx_msg.field.power = rx_msg.str.substring(indx1,indx2).toInt();
    indx1 = indx2+1;
    indx2 = rx_msg.str.indexOf(';',indx1+1);
    rx_msg.field.sf = rx_msg.str.substring(indx1,indx2).toInt();
    indx1 = indx2+1;
    indx2 = rx_msg.str.indexOf(';',indx1+1);
    rx_msg.field.remote_nbr = rx_msg.str.substring(indx1,indx2).toInt();
    indx1 = indx2+1;
    indx2 = rx_msg.str.indexOf('>',indx1+1);
    rx_msg.field.base_nbr = rx_msg.str.substring(indx1,indx2).toInt();
     
}

void parser_task(void)
{
    read_uart();
    if (rx_msg.status == STATUS_OK_FOR_ME)
    {
        parse_rx_frame();
        rx_msg.avail = false;
        rx_msg.status = STATUS_UNDEFINED;
    }
}
