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

    remote_uart_send_msg:           <UR2B;from;target;radio;freq;pwr;sf;rnbr;bnbr>\n
    remote_to_base_radio_msg:       <RR2B;from;target;radio;freq;pwr;sf;rnbr;bnbr>
    base_to_remote_radio_reply:     <RB2R;from;target;radio;freq;pwr;sf;rnbr;bnbr>
    remote_status:                  <USTA;from;target;radio;freq;pwr;sf;rnbr;bnbr>\n
    base_status:                    <USTA;from;target;radio;freq;pwr;sf;rnbr;bnbr>\n
    set_parameter:                  <USET;pindex;value>
    get_parameter:                  <UGET;pindex;
    parameter_value:                <UVAL;pindex;value>
    
    msg_id  = 4 charcters
    from    = from node address 1..127
    target  = target node address  0..127
    radio   = LoRa = 1 | RFM69 = 2
    freq    = 433 .. 868
    pwr     = power level:  5-20
    sf      = spreading factor
    rnbr    = remote send counter: 0-65000, incremented for each message sent by the remote
    bnbr    = base reply counter: 0-65000, incremented for each reply sent by the base
    pindx   = parameter index

  Examples:
    <RSND;1;2;L8;20;12;1234;1001>\n   (UART Command)
    <RS2B;1;2;L8;20;12;1234;1001>     (Radio Message from server )



*******************************************************************************

******************************************************************************/
#include <SPI.h>
#include "main.h"
#include "atask.h"
#include "io.h"
#include "rfm.h"
#include "uartx.h"


void print_debug_task(void);

//                                  123456789012345   ival  next  state  prev  cntr flag  call backup
atask_st debug_task_handle    =   {"Debug Task     ", 5000,    0,     0,  255,    0,  1,  print_debug_task };

main_ctrl_st main_ctrl = { 0, NODE_ROLE_UNDEFINED, false, false, 0 };

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

  io_initialize();
  //while (!main_ctrl.io_initialized){delay(100);}
  atask_initialize();
  atask_add_new(&debug_task_handle);
  uartx_initialize();
  rfm_initialize(main_ctrl.node_role);
}

void setup1(void)
{
    main_ctrl.io_initialized = true;

    #if BOARD == BOARD_T2504_PICO_RFM95_80x70
    io_blink(COLOR_RED, BLINK_OFF);
    io_blink(COLOR_GREEN, BLINK_ON);
    io_blink(COLOR_BLUE, BLINK_OFF);
    #endif
}
void loop()
{
  atask_run();

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

void print_debug_task(void)
{
    atask_print_status(true);
}

bool sema_reserve_serial(uint8_t task_id)
{
  if(main_ctrl.serial_reserved == 0)
  {
    main_ctrl.serial_reserved = task_id;
  }
  return (main_ctrl.serial_reserved == task_id);
}

void sema_wait_serial(uint8_t task_id)
{
    while (!sema_reserve_serial(task_id))
    {
        Serial.printf("wait: %d reserved %d\n",task_id, main_ctrl.serial_reserved);
    }
}

void sema_release_serial(void)
{
  main_ctrl.serial_reserved = 0;
}
