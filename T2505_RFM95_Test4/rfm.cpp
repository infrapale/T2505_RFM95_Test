
#include <SPI.h>
#include <RH_RF95.h>
#include "main.h"
#include "io.h"
#include "rfm.h"


// Singleton instance of the radio driver
//RH_RF95 rf95;
RH_RF95 rf95(PIN_RFM_CS, PIN_RFM_IRQ );
//RH_RF95 rf95(5, 2); // Rocket Scream Mini Ultra Pro with the RFM95W
//RH_RF95 rf95(8, 3); // Adafruit Feather M0 with RFM95 

rfm_ctrl_st rfm_ctrl;


//                                  123456789012345   ival  next  state  prev  cntr flag  call backup
atask_st rfm_task_handle      =  {"RFM Task       ", 100,    0,     0,  255,    0,  1,  rfm_task };



void rfm_initialize(node_role_et node_role)
{
    if (rf95.init())
    {
        rfm_ctrl.node_role = node_role;
        rfm_ctrl.tindx =  atask_add_new(&rfm_task_handle);
    }
    else
    {
        rfm_ctrl.node_role = NODE_ROLE_UNDEFINED;
        Serial.println("init failed");
    }
}

uint32_t rfm_timeout;
uint8_t  data[] = "Hello World!";
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t len = sizeof(buf);

void loop_client(void)
{
    switch(rfm_task_handle.state)
    {
        case 0:
            rfm_task_handle.state = 10;
            break;
        case 10:
            Serial.println("Sending to rf95_server");
            // Send a message to rf95_server
            rf95.send(data, sizeof(data));
            rf95.waitPacketSent();
            // Now wait for a reply
            rfm_timeout = millis() + 3000;
            atask_delay( rfm_ctrl.tindx, 10);  //Shorten run interval
            if (rf95.waitAvailableTimeout(1000))
            {
                rfm_task_handle.state = 30;
            }
            break;
        case 30:
            // Should be a reply message for us now   
            if (rf95.recv(buf, &len))
            {
                rfm_ctrl.rssi = rf95.lastRssi();
                Serial.print("got reply: ");
                Serial.println((char*)buf);
                Serial.print("RSSI: ");
                Serial.println(rfm_ctrl.rssi, DEC);    
            }
            else
            {
                Serial.println("recv failed");
            }
            rfm_task_handle.state = 50;  
            break;
        case 40:
            Serial.println("No reply, is rf95_server running?");
            rfm_task_handle.state = 50;  
            break;
        case 50:    ; 
            rfm_timeout = millis() + 5000;
            rfm_task_handle.state = 60;  
            break;
        case 60:
            if(millis() > rfm_timeout) rfm_task_handle.state = 100; 
            break;
        case 100:
            rfm_task_handle.state = 0; 
            break;

    }
}

void loop_server(void)
{
  if (rf95.available())
  {
    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len))
    {
      //digitalWrite(led, HIGH);
//      RH_RF95::printBuffer("request: ", buf, len);
      Serial.print("got request: ");
      Serial.println((char*)buf);
//      Serial.print("RSSI: ");
//      Serial.println(rf95.lastRssi(), DEC);
      
      // Send a reply
      uint8_t data[] = "And hello back to you";
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
      Serial.println("Sent a reply");
       //digitalWrite(led, LOW);
    }
    else
    {
      Serial.println("recv failed");
    }
  }
}

void rfm_task(void)
{   
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


