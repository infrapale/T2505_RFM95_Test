/*
https://www.airspayce.com/mikem/arduino/RadioHead/classRH__RF95.html
https://learn.adafruit.com/feather-rp2040-rfm95/arduino-ide-setup
https://www.airspayce.com/mikem/arduino/RadioHead/

*/

#include <SPI.h>
#include <RH_RF95.h>
#include "main.h"
#include "io.h"
#include "rfm.h"
#include "parser.h"


// Singleton instance of the radio driver
//RH_RF95 rf95;
RH_RF95 rf95(PIN_RFM_CS, PIN_RFM_IRQ );
//RH_RF95 rf95(5, 2); // Rocket Scream Mini Ultra Pro with the RFM95W
//RH_RF95 rf95(8, 3); // Adafruit Feather M0 with RFM95 

rfm_ctrl_st rfm_ctrl = {0};
extern main_ctrl_st main_ctrl;
//                                  123456789012345   ival  next  state  prev  cntr flag  call backup
atask_st rfm_task_handle      =  {"RFM Task       ", 10,    0,     0,  255,    0,  1,  rfm_task };


void rfm_initialize(node_role_et node_role)
{

    rfm_ctrl.rx_msg.avail = false;
    if (rf95.init())
    {
        rfm_ctrl.node_role = node_role;
        rfm_set_frequency(868.8);
        rfm_set_power(20);
    }
    else
    {
        rfm_ctrl.node_role = NODE_ROLE_UNDEFINED;
        Serial.println("init failed");
    }
    rfm_ctrl.rec_msg_len    = sizeof(rfm_ctrl.rec_msg);
    rfm_ctrl.send_msg_len   = sizeof(rfm_ctrl.send_msg);
}

void rfm_task_initilaize(void)
{
    rfm_ctrl.tindx =  atask_add_new(&rfm_task_handle);
}

void rfm_reset(void)
{
    digitalWrite(PIN_RFM_RESET, LOW);
    delay(100);
    digitalWrite(PIN_RFM_RESET, HIGH);
    delay(100);
    rfm_initialize(main_ctrl.node_role);
} 

uint32_t rfm_timeout;
uint8_t sdata[] = "And hello back to you";


void rfm_send_str(char *msg)
{
   uint8_t msg_len = strlen(msg);
   memset(rfm_ctrl.send_msg,0x00,RH_RF95_MAX_MESSAGE_LEN);
   memcpy(rfm_ctrl.send_msg, msg, msg_len);
   rfm_ctrl.send_msg_len = msg_len; 
}

void rfm_set_power(int8_t pwr)
{
    rfm_ctrl.power =  pwr;
    rf95.setTxPower( pwr);
}

void rfm_set_frequency(float freq)
{
    rfm_ctrl.frequency = freq;
    rf95.setFrequency(freq);
}

void rfm_set_sf(uint8_t sf)
{
    rfm_ctrl.sf  = sf;
    rf95.setSpreadingFactor(sf);
}

uint8_t data[] = "Hello World!";

void fix_serial1(void)
{
    //Serial1.end();
    //Serial1.begin(INTERCONNECT_UART_BPS);
    //delay(1000);
    Serial1.clearWriteError();
    Serial1.setPollingMode(false);
}
void loop_client(void)
{
    //Serial.print("C"); Serial.print(rfm_task_handle.state);
    switch(rfm_task_handle.state)
    {
        case 0:
            rfm_task_handle.state = 5;
            break;
        case 5:    
            if(rfm_ctrl.send_msg_len > 0) 
            {
                rfm_ctrl.reply_status =  REPLY_UNDEFINED;
                rfm_task_handle.state = 10;
                io_blink(COLOR_BLUE, BLINK_FAST_BLINK);
            }
            break;
        case 10:
            // Serial.printf("Sending to rf95_server: %d\n", rfm_ctrl.send_data_len);
            //delay(100);
            //Serial1.println((char*)rfm_ctrl.send_msg);
            //Serial1.flush();
            //delay(100);
            // Send a message to rf95_server
            //rf95.send(data, sizeof(data));
            rf95.send(rfm_ctrl.send_msg, rfm_ctrl.send_msg_len);
            //delay(100);
            rf95.waitPacketSent();
            //delay(100);
            //Serial1.flush();
            rfm_ctrl.send_msg_len = 0;
            // Now wait for a reply
            //rfm_timeout = millis() + 3000;
            //atask_delay( rfm_ctrl.tindx, 10);  //Shorten run interval           
            if (rf95.waitAvailableTimeout(3000))
            {
                //Serial1.print("< ");
                rfm_ctrl.rec_msg_len = RH_RF95_MAX_MESSAGE_LEN-10;
                rfm_task_handle.state = 30;
                //Serial1.println("new state 30");
        }
            else
            {
              rfm_task_handle.state = 40;
              rfm_ctrl.reply_status =  REPLY_FAILED;
              //fix_serial1();
              Serial1.println("*******REPLY_FAILED*******");
            }
            break;
        case 30:
            // Should be a reply message for us now   
            if (rf95.recv(rfm_ctrl.rec_msg, &rfm_ctrl.rec_msg_len))
            {
                rfm_ctrl.reply_status =  REPLY_RECEIVED;
                rfm_ctrl.rssi = rf95.lastRssi();
                if(rfm_ctrl.rec_msg_len < RH_RF95_MAX_MESSAGE_LEN-1 )
                {
                    rfm_ctrl.rec_msg[rfm_ctrl.rec_msg_len] = 0x00;
                    Serial1.println((char*)rfm_ctrl.rec_msg);
                }
                parser_radio_reply(rfm_ctrl.rec_msg, rfm_ctrl.rssi);
                //Serial1.print("got reply: ");
                //Serial.println((char*)rfm_ctrl.rec_msg);
                //Serial.print("RSSI: ");
                //Serial.println(rfm_ctrl.rssi, DEC);    
                rfm_task_handle.state = 50;  
            }
            else
            {
                //Serial.println("recv failed");
                rfm_task_handle.state = 40;  
                rfm_ctrl.reply_status =  REPLY_FAILED;
            }            
            break;
        case 40:
            io_blink(COLOR_RED, BLINK_FAST_FLASH);           
            Serial.println("No reply, is rf95_server running?");
            rfm_task_handle.state = 50;  
            break;
        case 50:    
            io_blink(COLOR_BLUE, BLINK_OFF); 
            rfm_timeout = millis() + 1000;
            rfm_task_handle.state = 60;  
            break;
        case 60:
            if(millis() > rfm_timeout) rfm_task_handle.state = 100; 
            break;
        case 100:
            io_blink(COLOR_RED, BLINK_OFF);
            rfm_task_handle.state = 0; 
            break;

    }
}

void loop_server(void)
{
    //Serial.print("S"); Serial.print(rfm_task_handle.state);
    char txt[40];
    switch(rfm_task_handle.state)
    {
        case 0:
            rfm_task_handle.state = 10;
            rfm_ctrl.rec_msg_len = RH_RF95_MAX_MESSAGE_LEN-10;
            break;
        case 10:
            if (rf95.available())
            {
                // Should be a message for us now   
                if (rf95.recv(rfm_ctrl.rec_msg, &rfm_ctrl.rec_msg_len))
                {
                    rfm_ctrl.reply_status =  REPLY_RECEIVED;
                    rfm_ctrl.rssi = rf95.lastRssi();
                    if(rfm_ctrl.rec_msg_len < RH_RF95_MAX_MESSAGE_LEN-1 )
                    {
                        rfm_ctrl.rec_msg[rfm_ctrl.rec_msg_len] = 0x00;
                        Serial1.println((char*)rfm_ctrl.rec_msg);
                    }
                    parser_radio_reply(rfm_ctrl.rec_msg, rfm_ctrl.rssi);
                    rfm_ctrl.server_cntr++;


                  rfm_ctrl.rssi = rf95.lastRssi(); 
                  io_blink(COLOR_BLUE, BLINK_FAST_BLINK);
                  Serial.print("got request: ");
                  Serial.println((char*)rfm_ctrl.rec_msg);
                  Serial.print("RSSI: ");
                  Serial.println(rfm_ctrl.rssi, DEC);
                  rfm_task_handle.state = 20;
                }
                else 
                {
                  io_blink(COLOR_RED, BLINK_FAST_FLASH); 
                  rfm_task_handle.state = 50;
                }
            }
            break;
        case 20:
            // Send a reply
            sprintf(txt,"<RREP;%d;%d;%d;%d;%d;%d;%d;%d>",
                rfm_ctrl.rx_msg.field.from,
                rfm_ctrl.rx_msg.field.target,
                rfm_ctrl.node_role,
                rfm_ctrl.power,
                rfm_ctrl.rssi,
                rfm_ctrl.sf,
                rfm_ctrl.rx_msg.field.remote_nbr,
                rfm_ctrl.server_cntr
                );
            rfm_ctrl.send_msg_len = strlen(txt); 
            memcpy(rfm_ctrl.send_msg, txt, rfm_ctrl.send_msg_len);
            //memcpy(rfm_ctrl.send_msg, "<RREP;1;2;3;14;-77;12;33;444>",RH_RF95_MAX_MESSAGE_LEN);
            rf95.send(rfm_ctrl.send_msg, rfm_ctrl.send_msg_len);
            rf95.waitPacketSent();
            Serial.println("Sent a reply");
            rfm_task_handle.state = 100;
            break;
        case 50:
            Serial.println("recv failed");
            rfm_task_handle.state = 100;
            break;
        case 100:
            io_blink(COLOR_BLUE, BLINK_OFF);
            io_blink(COLOR_RED, BLINK_OFF);
            rfm_task_handle.state = 10;
            break;

    }
}  

void rfm_task(void)
{   
    //Serial.print("-");
    //Serial1.print("|");
    switch(rfm_ctrl.node_role)
    {
        case NODE_ROLE_CLIENT:
            loop_client();
            break;
        case NODE_ROLE_SERVER:
            loop_server();
            break;
        default:
            Serial.print("No radio role defined");
        break;

    }
}


