/******************************************************************************
T2505_RFM95_Test2
*******************************************************************************

-------------       --------------       --------------       --------------
| Remote    |       |  Remote    |       |  Base      |       |  Base      |
| Client    |       |  Client    |       |  Server    |       |  Server    |
| Master    |       |  RFM       |       |  RFM       |       |  Master    |
-------------       --------------       --------------       --------------
      |                   |                    |                    |
      |  remote_cmd       |                    |                    |
      |------------------>|                    |                    |
      |                   | remote_base_msg    |                    |
      |                   |------------------->|                    |
      |                   |                    |                    |
      |                   | base_remote_rpl    |                    |
      |                   |<------------------ |                    |
      |                   |                    |                    |
      |                   |                    | base_status        |
      |                   |                    |------------------->|
      |  remote_status    |                    |                    |
      |<------------------|                    |                    |
      |                   |                    |                    |
      |                   |                    |                    |
      |                   |                    |                    |

    remote_msg:         <;RSND;from;target;radio;pwr;sf;rnbr;bnbr;>;\n
    remote_base_msg:    <;RS2B;from;target;radio;pwr;sf;rnbr;bnbr;>;
    base_remote_rpl:    <;BR2R;from;target;radio;pwr;sf;rnbr;bnbr;>;
    remote_status:      <;RSTA;from;target;radio;pwr;sf;rnbr;bnbr;>;\n
    base_status:        <;BSTA;from;target;radio;pwr;sf;rnbr;bnbr;>;\n

    msg_id  = 4 charcters
    from    = from node address 1..127
    target  = target node address  0..127
    radio   = L4 | L8 | R4
    pwr     = power level:  5-20
    sf      = spreading factor
    rnbr    = remote send counter: 0-65000, incremented for each message sent by the remote
    bnbr    = base reply counter: 0-65000, incremented for each reply sent by the base

  Examples:
    <;RSND;1;2;L8;20;12;1234;1001;>\n
    <;RS2B;1;2;L8;20;12;1234;1001;>;



*******************************************************************************

******************************************************************************/
#include <SPI.h>
#include "main.h"
#include "atask.h"
#include "io.h"
#include "rfm.h"


void print_debug_task(void);

//                                  123456789012345   ival  next  state  prev  cntr flag  call backup
atask_st debug_task_handle    =   {"Debug Task     ", 5000,    0,     0,  255,    0,  1,  print_debug_task };

main_ctrl_st main_ctrl = { 0, NODE_ROLE_UNDEFINED, false };

void setup() 
{
  Serial1.setTX(PIN_UART0_TX);   // UART0
  Serial1.setRX(PIN_UART0_RX);
  Serial2.setTX(PIN_UART1_TX);   // UART1
  Serial2.setRX(PIN_UART1_RX);
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600);

  uint8_t sw_bm = io_get_switch_bm();
  if ((sw_bm & SW_BM_TEST) == 0)
  {
    main_ctrl.test_activated = true;
    while (!Serial) ; // Wait for serial port to be available
    // deactivate watchdog
    delay(2000);

  }
  Serial.print("T2505_RFM95_Test"); Serial.print(" Compiled: ");
  Serial.print(__DATE__); Serial.print(" ");
  Serial.print(__TIME__); Serial.println();
  if(main_ctrl.test_activated) Serial.println("Test Mode is Activated");
  if ((sw_bm & SW_BM_ROLE) != 0)  main_ctrl.node_role = NODE_ROLE_CLIENT;
  else main_ctrl.node_role = NODE_ROLE_SERVER;
  main_ctrl.node_addr =  sw_bm & SW_BM_ADDR;
  Serial.printf("Node Address %d\n", main_ctrl.node_addr);

  delay(1000);  //ensure that IO was initialized
  
  rfm_initialize(main_ctrl.node_role);
  

  //atask_initialize();
  //atask_add_new(&debug_task_handle);
  //io_initialize();

}

void setup1(void)
{
    io_initialize();
    #if BOARD == BOARD_T2504_PICO_RFM95_80x70
    io_blink(COLOR_RED, BLINK_OFF);
    io_blink(COLOR_GREEN, BLINK_ON);
    io_blink(COLOR_BLUE, BLINK_OFF);
    #endif
}
void loop()
{
  rfm_loop();
}

uint32_t io_run_time = millis();
void loop1()
{
    if(millis() > io_run_time)
    {
        io_run_time = millis() + 100;
        io_task();
    }
    //Serial1.println("Print to UART0 TX");
    if (Serial1.available())
    {
        String  rx_str;
        rx_str = Serial1.readStringUntil('\n');
        Serial1.print("RFM95 Unit Received: ");
        Serial1.println(rx_str);
    }

}

void print_debug_task(void)
{
  atask_print_status(true);

}

