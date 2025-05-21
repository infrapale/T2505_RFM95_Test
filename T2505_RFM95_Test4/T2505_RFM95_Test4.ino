
#include <SPI.h>
#include <RH_RF95.h>
#include "rfm.h"
#include "main.h"
#include "io.h"

// #define PIN_RFM_MOSI        (19u)
// #define PIN_RFM_MISO        (16u)
// #define PIN_RFM_SCK         (18u)
// #define PIN_RFM_CS          (17u)
// #define PIN_RFM_IRQ         (21u)
// #define PIN_RFM_RESET       (20u)


// Singleton instance of the radio driver
//RH_RF95 rf95;
//RH_RF95 rf95(PIN_RFM_CS, PIN_RFM_IRQ );
//RH_RF95 rf95(5, 2); // Rocket Scream Mini Ultra Pro with the RFM95W
//RH_RF95 rf95(8, 3); // Adafruit Feather M0 with RFM95 

// Need this on Arduino Zero with SerialUSB port (eg RocketScream Mini Ultra Pro)
//#define Serial SerialUSB


main_ctrl_st main_ctrl = { 0, NODE_ROLE_UNDEFINED, false, false, 0 };


void setup() 
{
  // Rocket Scream Mini Ultra Pro with the RFM95W only:
  // Ensure serial flash is not interfering with radio communication on SPI bus
//  pinMode(4, OUTPUT);
//  digitalWrite(4, HIGH);

  Serial.begin(9600);
  io_initialize();
  uint8_t sw_bm = io_get_switch_bm();
  if ((sw_bm & SW_BM_TEST) == 0)
  {
    main_ctrl.test_activated = true;
    while (!Serial) ; // Wait for serial port to be available
    // deactivate watchdog
    delay(2000);
  }

  //while (!Serial) ; // Wait for serial port to be available
  Serial.print("T2505_RFM95_Test3"); Serial.print(" Compiled: ");
  Serial.print(__DATE__); Serial.print(" ");
  Serial.print(__TIME__); Serial.println();
  if(main_ctrl.test_activated) Serial.println("Test Mode is Activated");
  if ((sw_bm & SW_BM_ROLE) != 0)  
  {
    Serial.print("Client ");
    main_ctrl.node_role = NODE_ROLE_CLIENT;
  }
  else 
  {
    Serial.print("Server ");
    main_ctrl.node_role = NODE_ROLE_SERVER;
  }  
  main_ctrl.node_addr =  sw_bm & SW_BM_ADDR;
  Serial.printf("Node Address %d\n", main_ctrl.node_addr);

  rfm_initialize(main_ctrl.node_role); 
}
void setup1(void)
{
    #if BOARD == BOARD_T2504_PICO_RFM95_80x70
    io_blink(COLOR_RED, BLINK_OFF);
    io_blink(COLOR_GREEN, BLINK_BLINK);
    io_blink(COLOR_BLUE, BLINK_OFF);
    #endif
}

void loop()
{   
  atask_run();
  //rfm_task();
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

