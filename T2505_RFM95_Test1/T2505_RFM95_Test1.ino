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


// #define DEFAULT_ADDRESS 0
// #define CLIENT_ADDRESS 1
// #define SERVER_ADDRESS 2


// Singleton instance of the radio driver
// RH_RF95(uint8_t slaveSelectPin = SS, uint8_t interruptPin = 2, RHGenericSPI& spi = hardware_spi);
//RH_RF95 driver();
RH_RF95 driver(PIN_RFM_CS, PIN_RFM_IRQ );
//RH_RF95 driver(5, 2); // Rocket Scream Mini Ultra Pro with the RFM95W

//Class to manage message delivery and receipt, using the driver declared above
//RHReliableDatagram manager(driver, CLIENT_ADDRESS);
RHReliableDatagram *managerp;
// #ifdef LORA_CLIENT
// RHReliableDatagram manager(driver, CLIENT_ADDRESS);
// node_ctrl_st node_ctrl = {NODE_CLIENT};
// #else
// RHReliableDatagram manager(driver, SERVER_ADDRESS);
// node_ctrl_st node_ctrl = {NODE_SERVER};
// #endif

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

main_ctrl_st main_ctrl = { 0, NODE_ROLE_UNDEFINED, 0, 0};

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
  Serial.print("T2505_RFM95_Test"); Serial.print(" Compiled: ");
  Serial.print(__DATE__); Serial.print(" ");
  Serial.print(__TIME__); Serial.println();

  main_ctrl.node_addr = io_get_switch_bm();
  if((main_ctrl.node_addr & 0b00001000) != 0) main_ctrl.node_role = NODE_ROLE_CLIENT;
  else main_ctrl.node_role = NODE_ROLE_SERVER;
  main_ctrl.node_addr &= ~0b00001000;

  delay(1000);  //ensure that IO was initialized
  switch(main_ctrl.node_role)
  {
    case NODE_ROLE_CLIENT: 
      static RHReliableDatagram client_manager(driver, CLIENT_ADDRESS);
      managerp = &client_manager;
      Serial.print("LoRa Client Node ");
      io_blink(COLOR_GREEN, 5);
      break;
    case NODE_ROLE_SERVER:
      static RHReliableDatagram server_manager(driver, SERVER_ADDRESS);
      managerp = &server_manager;
      io_blink(COLOR_GREEN, 6);
      Serial.print("LoRa Server Node ");
      break;
    default:
      Serial.println("Node Role was not defined!!");
      io_blink(COLOR_RED, 5);
      io_blink(COLOR_GREEN, 0);
      io_blink(COLOR_BLUE, 5);
      while(true){};
      break; 
  } 
  Serial.printf("Node Address %d\n", main_ctrl.node_addr);

  //atask_initialize();
  //atask_add_new(&debug_task_handle);
  //io_initialize();

  //driver.setPreambleLength(uint16_t bytes);
  driver.setFrequency(868.0);

  if (!managerp->init()) Serial.println("RFM95 init failed");
  else Serial.println("RFM95 was Initialized");

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
    io_blink(COLOR_RED, 0);
    io_blink(COLOR_GREEN, 2);
    io_blink(COLOR_BLUE, 0);
    #endif
}

void loop_client()
{
    Serial.println("Sending to rf95_reliable_datagram_server");
      
    // Send a message to manager_server
    sprintf((char*)data[0],"C->S;%05d;%05d",main_ctrl.client_cntr,main_ctrl.server_cntr);
    if (managerp->sendtoWait(data[0], sizeof(data[0]), SERVER_ADDRESS))
    {
      main_ctrl.client_cntr++;
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

void loop_server()
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
          sprintf((char*)data[1],"S->C;%05d;%05d",main_ctrl.client_cntr,main_ctrl.server_cntr);
          if (managerp->sendtoWait(data[1], sizeof(data[1]), from))
            main_ctrl.server_cntr = main_ctrl.client_cntr;
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

void loop()
{
  switch(main_ctrl.node_role)
  {
      case NODE_ROLE_CLIENT:
        loop_client();
        break;
      case NODE_ROLE_SERVER:
        loop_server();
        break;
      default:    
        Serial.println("Incorrect Node Role!!");
        delay(5000);
        break;
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

