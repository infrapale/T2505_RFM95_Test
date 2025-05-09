// rf95_reliable_datagram_client.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple addressed, reliable messaging client
// with the RHReliableDatagram class, using the RH_RF95 driver to control a RF95 radio.
// It is designed to work with the other example rf95_reliable_datagram_server
// Tested with Anarduino MiniWirelessLoRa, Rocket Scream Mini Ultra Pro with the RFM95W 

#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>

#include "main.h"
#include "rfm.h"
#include "io.h"



// Singleton instance of the radio rf95
// RH_RF95(uint8_t slaveSelectPin = SS, uint8_t interruptPin = 2, RHGenericSPI& spi = hardware_spi);
//RH_RF95 rf95();
RH_RF95 rf95(PIN_RFM_CS, PIN_RFM_IRQ );
//RH_RF95 rf95(5, 2); // Rocket Scream Mini Ultra Pro with the RFM95W

//Class to manage message delivery and receipt, using the rf95 declared above
//RHReliableDatagram manager(rf95, CLIENT_ADDRESS);
RHReliableDatagram *managerp;
// #ifdef LORA_CLIENT
// RHReliableDatagram manager(rf95, CLIENT_ADDRESS);
// node_ctrl_st node_ctrl = {NODE_CLIENT};
// #else
// RHReliableDatagram manager(rf95, SERVER_ADDRESS);
// node_ctrl_st node_ctrl = {NODE_SERVER};
// #endif


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
    case NODE_ROLE_RELIABLE_CLIENT: 
      static RHReliableDatagram client_manager(rf95, CLIENT_ADDRESS);
      managerp = &client_manager;
      Serial.print("LoRa Reliable Client Node ");
      io_blink(COLOR_GREEN, BLINK_RELIABLE_CLIENT);
      break;
    case NODE_ROLE_RELIABLE_SERVER:
      static RHReliableDatagram server_manager(rf95, SERVER_ADDRESS);
      managerp = &server_manager;
      io_blink(COLOR_GREEN, BLINK_RELIABLE_SERVER);
      Serial.print("LoRa Reliable Server Node ");
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
    if ((rfm_ctrl.node_role == NODE_ROLE_RELIABLE_CLIENT ) || (rfm_ctrl.node_role == NODE_ROLE_RELIABLE_SERVER ))
    {
      if (!managerp->init()) Serial.println("RFM95 manager init failed");
      else Serial.println("RFM95 Manager was Initialized");
    }
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

  sprintf((char*)data,"C->S;%05d;%05d",rfm_ctrl.client_cntr,rfm_ctrl.server_cntr);
  Serial.println((char*)data);
  rf95.send(data, sizeof(data));
  rfm_ctrl.client_cntr++;
  rf95.waitPacketSent();
  // Now wait for a reply
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

void loop_reliable_client()
{
    Serial.println("Sending to rf95_reliable_datagram_server");
      
    // Send a message to manager_server
    sprintf((char*)data[0],"C->S;%05d;%05d",rfm_ctrl.client_cntr,rfm_ctrl.server_cntr);
    if (managerp->sendtoWait(data[0], sizeof(data[0]), SERVER_ADDRESS))
    {
      rfm_ctrl.client_cntr++;
      // Now wait for a reply from the server
      uint8_t len = sizeof(buf);
      uint8_t from;   
      if (managerp->recvfromAckTimeout(buf, &len, 2000, &from))
      {
        Serial.printf("Got Reply from : 0x%02X : %s\n", from, (char*)buf);
      }
      else
      {
        Serial.println("No reply, is rf95_reliable_datagram_server running?");
      }
    }
    else
      Serial.println("sendtoWait failed");
    delay(5000);
}

void loop_reliable_server()
{
      if (managerp->available())
      {
        // Wait for a message addressed to us from the client
        uint8_t len = sizeof(buf);
        uint8_t from;
        if (managerp->recvfromAck(buf, &len, &from))
        {
          String Buff = (char*)buf;
          String Str = Buff.substring(6,10);

          Serial.printf("Got Request from : 0x%02X : %s\n", from, (char*)buf);
          // Send a reply back to the originator client
          sprintf((char*)data[1],"S->C;%05d;%05d",rfm_ctrl.client_cntr,rfm_ctrl.server_cntr);
          if (managerp->sendtoWait(data[1], sizeof(data[1]), from))
            rfm_ctrl.server_cntr = rfm_ctrl.client_cntr;
          else
            Serial.println("sendtoWait failed");
        }
      }
      else
      {
        // Serial.println("Manager is not available");
      }
    delay(100);
}

void rfm_loop(void)
{

  switch(rfm_ctrl.node_role)
  {
      case NODE_ROLE_CLIENT:
        loop_client();
        break;
      case NODE_ROLE_SERVER:
        loop_server();
        break;
      case NODE_ROLE_RELIABLE_CLIENT:
        loop_client();
        break;
      case NODE_ROLE_RELIABLE_SERVER:
        loop_server();
        break;
      default:    
        Serial.println("Incorrect Node Role!!");
        delay(5000);
        break;
  }
}

