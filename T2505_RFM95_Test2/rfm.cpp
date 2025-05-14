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
atask_st rfm_task_handle       =  {"RFM  Task      ",100,    0,     0,  255,    0,  1,  rfm_task };



//rfm_ctrl_st rfm_ctrl = { 0, NODE_ROLE_UNDEFINED, 0, 0, NULL, 0};
rfm_ctrl_st rfm_ctrl = 
{ 
  .node_addr= 0, 
  .node_role = NODE_ROLE_UNDEFINED, 
  .client_cntr = 0, 
  .server_cntr = 0, 
  .tindx = 0,
  .taskp = NULL, 
  .tatio = 0
};

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
    rfm_ctrl.taskp = atask_get_task(rfm_ctrl.tindx);

   }
  else
  {
     Serial.println("RFM95 init failed");
     io_blink(COLOR_RED, BLINK_SHORT_FLASH);
  }
 
}

void loop_client()
{
  static uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  static uint8_t len = sizeof(buf);
  // Send a message to rf95_server
  static uint8_t data[40];
  switch(rfm_ctrl.taskp->state)
  {
    case 0:
      io_blink(COLOR_BLUE, BLINK_ON);
      rfm_ctrl.tatio = millis() + 1000;
      rfm_ctrl.taskp->state = 10;
      break;
    case 10:
      if (millis() > rfm_ctrl.tatio)
      {
        io_blink(COLOR_BLUE, BLINK_FAST_FLASH);
        sprintf((char*)data,"C->S;%05d;%05d",rfm_ctrl.client_cntr,rfm_ctrl.server_cntr);
        //Serial.println((char*)data);
        rf95.send(data, sizeof(data));
        rfm_ctrl.client_cntr++;
        // Now wait for a reply
        //rfm_ctrl.taskp->state = 20;
      }
      //break;
    //case 20:
      //Serial.println("Wait for Packet to be Sent");
      rf95.waitPacketSent();
      //Serial.flush();
      io_blink(COLOR_BLUE, BLINK_SHORT_FLASH);
      //Serial.println("Packet was sent, wait for reply");
      //rfm_ctrl.tatio = millis() + 3000;
      //rfm_ctrl.taskp->state = 30;
      //break;
    //case 30:
      if (rf95.waitAvailableTimeout(4000))
      { 
        // Should be a reply message for us now   
        if (rf95.recv(buf, &len))
        {
            Serial.printf("Received Reply: %s, RSSI= %d\n",(char*)buf,rf95.lastRssi());
            io_blink(COLOR_GREEN, BLINK_ON);
            rfm_ctrl.tatio = millis() + 5000;
            rfm_ctrl.taskp->state = 40;
        }
        else
        {
          Serial.println("recv failed");
          io_blink(COLOR_RED, BLINK_FAST_FLASH);          
          rfm_ctrl.tatio = millis() + 5000;
          rfm_ctrl.taskp->state = 100;
        }
      }
      else 
      {
        //if (millis() > rfm_ctrl.tatio)
        {
          Serial.println("No reply, is rf95_server running?");
          io_blink(COLOR_RED, BLINK_ON);
          rfm_ctrl.tatio = millis() + 5000;
          rfm_ctrl.taskp->state = 100;
        }
      }

      break;
    case 40:
      if (millis() > rfm_ctrl.tatio) rfm_ctrl.taskp->state = 200;
      break;
    case 100:
      if (millis() > rfm_ctrl.tatio) rfm_ctrl.taskp->state = 200;
      break;
    case 200:   // return to start
        io_blink(COLOR_RED, BLINK_OFF);
        io_blink(COLOR_GREEN, BLINK_OFF);
        io_blink(COLOR_BLUE, BLINK_OFF);
        rfm_ctrl.taskp->state = 0;
        break;
  }
  if (rfm_ctrl.taskp->prev_state != rfm_ctrl.taskp->state)
    Serial.printf("loop client state: %d -> %d\n", rfm_ctrl.taskp->prev_state, rfm_ctrl.taskp->state);
  Serial.flush();


  /*
  io_blink(COLOR_BLUE, BLINK_FAST_FLASH);
  sprintf((char*)data,"C->S;%05d;%05d",rfm_ctrl.client_cntr,rfm_ctrl.server_cntr);
  Serial.println((char*)data);
  rf95.send(data, sizeof(data));
  rfm_ctrl.client_cntr++;
 // Now wait for a reply
  Serial.println("Wait for Packet to be Sent");
  rf95.waitPacketSent();
  Serial.flush();
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
    io_blink(COLOR_RED, BLINK_FAST_FLASH);
  }
  delay(4000);
  */
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

