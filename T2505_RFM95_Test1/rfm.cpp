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
#include "io.h"


extern main_ctrl_st main_ctrl;
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


