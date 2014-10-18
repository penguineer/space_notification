/*
 * Spacetime-Schalter-Controller mit I³C-Anbindung
 * Autor: Stefan Haun <tux@netz39.de>
 * 
 * Entwickelt für ATTINY85
 * 
 * nutzt https://github.com/eriksl/usitwislave.git
 * 
 * DO NOT forget to set the fuses s.th. the controller uses the 8 MHz clock!
 */


/* define CPU frequency in MHz here if not defined in Makefile */
#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/twi.h>
#include <stdint.h>

#include "usitwislave.h"


inline void setPortB(char mask) {
  PORTB = PORTB | mask;
}

inline void resetPortB(char mask) {
  PORTB = PORTB & ~mask; 
}

// internal space state
#define STATE_CLOSED  1
#define STATE_OPEN    2
#define STATE_UNKNOWN 3
volatile uint8_t switch_state=STATE_UNKNOWN;

inline uint8_t getState() {
  return switch_state;
}

inline void setState(uint8_t _state) {
  switch_state = _state;
}

// switch readout
uint8_t getSwitch(uint8_t state) {
  switch (switch_state) {
    case STATE_CLOSED:
      return (PINB & (1<<PB4)) ? 0 : 1;
      break;
    case STATE_OPEN: 
      return (PINB & (1<<PB3)) ? 0 : 1;
      break;
  }
  
  return 0;
}


//flag state change
inline void i3c_stateChange() {
  // port B1 as output
  DDRB |= (1 << PB1);
  // set to low
  PORTB &= ~(1 << PB1);
}

// put to listening mode
inline void i3c_tristate() {
  // port B1 as input
  DDRB &= ~(1 << PB1);
  // no pull-up
  PORTB &= ~(1 << PB1);
}

inline uint8_t i3c_state() {
  return (PINB & (1 << PB1)) >> PB1;
}

/*
 * I²C Datenformat:
 * 
 * PCCCDDDD
 * 
 * parity (P)
 * command (CCC)
 *      Reset	 0x0	Reset I³C state
 * 	GetState 0x01   Aktuelle Schalterstellung ausgeben
 * 	SetState 0x02   Status Schalterstellung setzen
 * 
 * data (DDDD)
 * 	nicht verwendet
 */
#define CMD_I3C_RESET 0x00
#define CMD_GETSTATE  0x01
#define CMD_SETSTATE  0x02

static void twi_callback(uint8_t buffer_size,
                         volatile uint8_t input_buffer_length, 
                         volatile const uint8_t *input_buffer,
                         volatile uint8_t *output_buffer_length, 
                         volatile uint8_t *output_buffer) {
  
  if (input_buffer_length) {
    const uint8_t parity = (input_buffer[0] & 0x80) >> 7;
    const uint8_t cmd  = (input_buffer[0] & 0x70) >> 4;
    const uint8_t data = input_buffer[0] & 0x0F;
    
    // check parity
    uint8_t v = input_buffer[0] & 0x7F;
    uint8_t c;
    for (c = 0; v; c++)
      v &= v-1;
    c &= 1;
    
    // some dummy output value, as 0 states an error
    uint8_t output=0;
    
    // only check if parity matches
    if (parity == c)
    switch (cmd) {
      case CMD_I3C_RESET: {
	i3c_tristate();
	output = 1;
	break;
      }
      case CMD_GETSTATE: {
	output = getState();
	break;
      }
      case CMD_SETSTATE: {
	if (data >= 1 && data <= 3) { 
	  setState(data);
	  output = 1;
	}
	break;
      }
    }

    *output_buffer_length = 2;
    output_buffer[0] = output;
    output_buffer[1] = ~(output);    
  }  
}

static void twi_idle_callback(void) {
  // void
}

void init(void) {
  /*
   * Pin-Config PortB:
   *   PB0: I2C SDA
   *   PB1: I3C INT
   *   PB2: I2C SDC
   *   PB3: On  (In)
   *   PB4: Off (In)
   */
  DDRB  = 0b11100111;
  // PullUp für Eingänge
  PORTB = 0b11111111;

  // activate the timer
  cli();
  
  CLKPR = (1 << CLKPCE);  /*  enable clock prescaler update       */
  CLKPR = 0;              /*  set clock to maximum                */
  TIFR &= ~(1 << TOV0);   /*  clear timer0 overflow interrupt flag    */
  TIMSK |= (1 << TOIE0);  /*  enable timer0 overflow interrupt        */
  //TCCR0B = (1 << CS02) | (1 << CS00);
  TCCR0B = (1 << CS00);

  sei();
  
  // beep once, we have no state
  i3c_tristate();
}

int main(void)
{
  // initialisieren
  init();

  // start TWI (I²C) slave mode
  usi_twi_slave(0x24, 0, &twi_callback, &twi_idle_callback);

  return 0;
}


void switchState(uint8_t _state) {
  if (_state != getState())
    i3c_stateChange();
  setState(_state);
}


// nach http://www.mikrocontroller.net/articles/Entprellung#Softwareentprellung
#define DECHATTER_COUNTER 50

uint8_t key_state;
uint8_t key_counter;

void dechatterKey() {
  // adjust manual key chatter
  uint8_t input = (getSwitch(STATE_CLOSED) << 1) +
                  (getSwitch(STATE_OPEN));
 
  if( input != key_state ) {
    key_counter--;
    if( key_counter == 0 ) {
      key_counter = DECHATTER_COUNTER;
      key_state = input;
      if( !input ) {
        // set the new state
        if (input & 0x02)
	  switchState(STATE_CLOSED);
	else if (input & 0x01)
	  switchState(STATE_OPEN);
      }
    }
  }
  else
    key_counter = DECHATTER_COUNTER;
}

ISR (TIM0_OVF_vect)
{
  // store state and disable interrupts
  const uint8_t _sreg = SREG;
  cli();
  
  dechatterKey();
  
  // restore state
  SREG = _sreg;
}
