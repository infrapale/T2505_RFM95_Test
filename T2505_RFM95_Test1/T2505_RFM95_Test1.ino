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
#include "atask.h"
#include "io.h"


#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2


// Singleton instance of the radio driver
// RH_RF95(uint8_t slaveSelectPin = SS, uint8_t interruptPin = 2, RHGenericSPI& spi = hardware_spi);
//RH_RF95 driver();
RH_RF95 driver(PIN_RFM_CS, PIN_RFM_IRQ );
//RH_RF95 driver(5, 2); // Rocket Scream Mini Ultra Pro with the RFM95W

// Class to manage message delivery and receipt, using the driver declared above
#ifdef LORA_CLIENT
RHReliableDatagram manager(driver, CLIENT_ADDRESS);
node_ctrl_st node_ctrl = {NODE_CLIENT};

#else
RHReliableDatagram manager(driver, SERVER_ADDRESS);
node_ctrl_st node_ctrl = {NODE_SERVER};
#endif

// Need this on Arduino Zero with SerialUSB port (eg RocketScream Mini Ultra Pro)
//#define Serial SerialUSB

void print_debug_task(void){atask_print_status(true);}

//                                  123456789012345   ival  next  state  prev  cntr flag  call backup
atask_st debug_task_handle    =   {"Debug Task     ", 5000,    0,     0,  255,    0,  1,  print_debug_task };
uint8_t data[2][32]=
{    
    //1234567890123456789012
    "Hello World!",
    "And hello back to you"
};

//uint8_t data[] = "Hello World!";
// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];



void setup() 
{
  // Rocket Scream Mini Ultra Pro with the RFM95W only:
  // Ensure serial flash is not interfering with radio communication on SPI bus
//  pinMode(4, OUTPUT);
//  digitalWrite(4, HIGH);

  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  if(node_ctrl.node_type == NODE_CLIENT)
  {
      Serial.println("LoRa Client Node");
  }
  else 
  {
      Serial.println("LoRa Server Node");
  }
  //atask_initialize();
  //atask_add_new(&debug_task_handle);
  //io_initialize();

  //driver.setPreambleLength(uint16_t bytes);
  driver.setFrequency(868.0);

  if (!manager.init())
    Serial.println("init failed");
  else
    Serial.println("RFM95 was Initialized");

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 2 to 20 dBm:
//  driver.setTxPower(20, false);
  // If you are using Modtronix inAir4 or inAir9, or any other module which uses the
  // transmitter RFO pins and not the PA_BOOST pins
  // then you can configure the power transmitter power for 0 to 15 dBm and with useRFO true. 
  // Failure to do that will result in extremely low transmit powers.
//  driver.setTxPower(14, true);

  // You can optionally require this module to wait until Channel Activity
  // Detection shows no activity on the channel before transmitting by setting
  // the CAD timeout to non-zero:
//  driver.setCADTimeout(10000);


}

void setup1(void)
{
    io_initialize();
    #if BOARD == BOARD_T2504_PICO_RFM95_80x70
    io_blink(COLOR_RED, 1);
    io_blink(COLOR_GREEN, 6);
    io_blink(COLOR_BLUE, 7);
    #endif
}


void loop()
{
  if(node_ctrl.node_type == NODE_CLIENT)
  {         
      Serial.println("Sending to rf95_reliable_datagram_server");
        
      // Send a message to manager_server
      if (manager.sendtoWait(data[0], sizeof(data[0]), SERVER_ADDRESS))
      {
        // Now wait for a reply from the server
        uint8_t len = sizeof(buf);
        uint8_t from;   
        if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
        {
          Serial.print("got reply from : 0x");
          Serial.print(from, HEX);
          Serial.print(": ");
          Serial.println((char*)buf);
        }
        else
        {
          Serial.println("No reply, is rf95_reliable_datagram_server running?");
        }
      }
      else
        Serial.println("sendtoWait failed");
      delay(2000);
  }
  else 
  {
      if (manager.available())
      {
        // Wait for a message addressed to us from the client
        uint8_t len = sizeof(buf);
        uint8_t from;
        if (manager.recvfromAck(buf, &len, &from))
        {
          Serial.print("got request from : 0x");
          Serial.print(from, HEX);
          Serial.print(": ");
          Serial.println((char*)buf);

          // Send a reply back to the originator client
          if (!manager.sendtoWait(data[1], sizeof(data[1]), from))
            Serial.println("sendtoWait failed");
        }
      }
    delay(100);
    // Serial.print(node_ctrl.node_type);
  }
}

uint32_t io_run_time = millis();
void loop1()
{
    if(millis() > io_run_time)
    {
        io_run_time = millis() + 100;
        io_task();
    }
}

