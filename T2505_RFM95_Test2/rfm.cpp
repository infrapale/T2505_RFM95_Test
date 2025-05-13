// rf95_reliable_datagram_client.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple addressed, reliable messaging client
// with the RHReliableDatagram class, using the RH_RF95 driver to control a RF95 radio.
// It is designed to work with the other example rf95_reliable_datagram_server
// Tested with Anarduino MiniWirelessLoRa, Rocket Scream Mini Ultra Pro with the RFM95W 

#include <RH_RF95.h>
#include <SPI.h>

#include "main.h"
#include "atask.h"
#include "rfm.h"
#include "io.h"



// Singleton instance of the radio rf95
// RH_RF95(uint8_t slaveSelectPin = SS, uint8_t interruptPin = 2, RHGenericSPI& spi = hardware_spi);
//RH_RF95 rf95();
RH_RF95 rf95(PIN_RFM_CS, PIN_RFM_IRQ );
//RH_RF95 rf95(5, 2); // Rocket Scream Mini Ultra Pro with the RFM95W

void rfm_task(void);

//                                  123456789012345   ival  next  state  prev  cntr flag  call backup
atask_st rfm_task_handle       =   {"RFM  Task      ", 100,    0,     0,  255,    0,  1,  rfm_task };



rfm_ctrl_st rfm_ctrl = { 0, NODE_ROLE_UNDEFINED, 0, 0};

uint8_t data[2][32]=
{    
    //1234567890123456789012
    "Hello World!",
    "And hello back to you"
};

// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];



void rfm_initialize(node_role_et node_role)
{
  rfm_ctrl.node_role = node_role;

  switch(rfm_ctrl.node_role)
  {
    case NODE_ROLE_CLIENT: 
      Serial.print("LoRa Client Node ");
      io_blink(COLOR_GREEN, BLINK_CLIENT);
      break;
    case NODE_ROLE_SERVER:
      io_blink(COLOR_GREEN, BLINK_SERVER);
      Serial.print("LoRa Server Node ");
      break;
    default:
      Serial.println("Node Role was not defined!!");
      io_blink(COLOR_RED, BLINK_FLASH);
      io_blink(COLOR_GREEN, BLINK_OFF);
      io_blink(COLOR_BLUE, BLINK_OFF);
      while(true){};
      break; 
  } 
  Serial.printf("Node Address %d\n", rfm_ctrl.node_addr);
  
  //rf95.setPreambleLength(uint16_t bytes);

  if (rf95.init())
  {
    Serial.println("RFM95 was Initialized");
    rf95.setFrequency(868.0);
    rfm_ctrl.tindx =  atask_add_new(&rfm_task_handle);
   }
  else
  {
     Serial.println("RFM95 init failed");
     io_blink(COLOR_RED, BLINK_SHORT_FLASH);
  }


  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 2 to 20 dBm:
//  rf95.setTxPower(20, false);
  // If you are using Modtronix inAir4 or inAir9, or any other module which uses the
  // transmitter RFO pins and not the PA_BOOST pins
  // then you can configure the power transmitter power for 0 to 15 dBm and with useRFO true. 
  // Failure to do that will result in extremely low transmit powers.
//  rf95.setTxPower(14, true);

  // You can optionally require this module to wait until Channel Activity
  // Detection shows no activity on the channel before transmitting by setting
  // the CAD timeout to non-zero:
//  rf95.setCADTimeout(10000);

}


void loop_client()
{
  
  // Send a message to rf95_server
  uint8_t data[40];
  io_blink(COLOR_BLUE, BLINK_FAST_FLASH);
  sprintf((char*)data,"C->S;%05d;%05d",rfm_ctrl.client_cntr,rfm_ctrl.server_cntr);
  Serial.println((char*)data);
  rf95.send(data, sizeof(data));
  rfm_ctrl.client_cntr++;
 // Now wait for a reply
  Serial.println("Wait for Packet to be Sent");
  rf95.waitPacketSent();

  io_blink(COLOR_BLUE, BLINK_SHORT_FLASH);
  Serial.println("Packet was sent, wait for reply");
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  if (rf95.waitAvailableTimeout(3000))
  { 
    // Should be a reply message for us now   
    if (rf95.recv(buf, &len))
   {
      Serial.printf("Received Reply: %s, RSSI= %d\n",(char*)buf,rf95.lastRssi());
    }
    else
    {
      Serial.println("recv failed");
    }
  }
  else
  {
    Serial.println("No reply, is rf95_server running?");
  }
  delay(4000);
}

void loop_server()
{
  if (rf95.available())
  {
    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len))
    {
      rfm_ctrl.server_cntr++;
      //digitalWrite(led, HIGH);
      Serial.printf("Received: %s, RSSI= %d\n",buf,rf95.lastRssi());
      
      // Send a reply
      uint8_t data[40] = "And hello back to you";
      sprintf((char*)data,"S->C;%05d;%05d",rfm_ctrl.client_cntr,rfm_ctrl.server_cntr);
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
      Serial.printf("Sent a Reply: %s\n",(char*)data);
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
        Serial.println("Incorrect Node Role!!");
        break;
  }
}

