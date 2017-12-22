#include "rf24l01.h"

/*
RF24L01 stm8s demo
MASTER
This code receives two 32 bit ints from a master, and sends the result of the basic
mathematic operations involving them.

This is a proof of concept and not a copy and use library.
Adapt this code to your application.

Erwan Martin
Fall 2013
ARENIB Delta
http://delta.arenib.org/

License: MIT

RF24L01 connector pinout:
GND    VCC
CE     CSN
SCK    MOSI
MISO   IRQ

Connections:
  PC3 -> CE
  PC4 -> CSN
  PC7 -> MISO
  PC6 -> MOSI
  PC5 -> SCK

*/

uint8_t mutex,flag_send,count=0;
uint8_t recv_data[1];
uint8_t data_sent[1];


typedef struct _data_to_send {
  uint32_t add;
  uint32_t sub;
  uint32_t mult;
  uint32_t div;
} data_to_send;
data_to_send to_send;

typedef struct _data_received {
  uint32_t op1;
  uint32_t op2;
} data_received;
data_received received;

void delay(uint32_t n)
{
    while(n>0) n--;
 }
int main( void ) {
  CLK_DeInit();
  CLK_SYSCLKConfig(CLK_PRESCALER_HSIDIV4);
  CLK_ClockSwitchConfig(CLK_SWITCHMODE_AUTO,CLK_SOURCE_HSI,DISABLE,CLK_CURRENTCLOCKSTATE_DISABLE);
  
  GPIO_Init(GPIOD,GPIO_PIN_3,GPIO_MODE_OUT_PP_LOW_FAST);
  GPIO_WriteHigh(GPIOD,GPIO_PIN_3);
  delay(10000);
  GPIO_WriteLow(GPIOD,GPIO_PIN_3);
  GPIO_Init(GPIOD,GPIO_PIN_2,GPIO_MODE_IN_PU_NO_IT);

  RF24L01_init();
  uint8_t rx_addr[5] = {0x04, 0xAD, 0x45, 0x98, 0x51};
  uint8_t tx_addr[5] = {0x08, 0x15, 0xd7, 0x36, 0x22};
  RF24L01_setup(tx_addr, rx_addr, 12);
  RF24L01_set_mode_RX();
  //IRQ
  GPIO_Init(
    GPIOB,
    GPIO_PIN_4,
    GPIO_MODE_IN_FL_IT
  );
  EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOB, EXTI_SENSITIVITY_FALL_ONLY);
  enableInterrupts();

  while (1) {

      mutex=0;
      while(!mutex);
      if (mutex == 145) {
        RF24L01_read_payload(recv_data, 1);
        if(recv_data[0]=='M') GPIO_WriteReverse(GPIOD,GPIO_PIN_3);
        delay(1000);
        mutex=0;
        halt();
      }
      
    }
}

INTERRUPT_HANDLER(EXTI_PORTB_IRQHandler, 4) {
  uint8_t sent_info;
 // mutex=1;
  if (sent_info = RF24L01_was_data_sent()) {
    //Packet was sent or max retries reached
    flag_send=1;
    mutex = sent_info;
    RF24L01_clear_interrupts();
    return;
  }

  if(RF24L01_is_data_available()) {
    //Packet was received
    //GPIO_WriteHigh(GPIOD,GPIO_PIN_3);
    mutex = 145;
    count++;
    RF24L01_clear_interrupts();
    return;
  }
  
  RF24L01_clear_interrupts();
}
