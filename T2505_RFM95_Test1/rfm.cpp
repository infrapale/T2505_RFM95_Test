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

typedef 


void rfm_initialize(node_role_et node_role)

  switch(main_ctrl.node_role)
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
  Serial.printf("Node Address %d\n", main_ctrl.node_addr);
