/*
 * Ampel-Controller mit I²C-Anbindung
 * Autor: Stefan Haun <tux@netz39.de>
 * 
 * Entwickelt für ATTINY25
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

const uint8_t COLOR_NONE  = 0;
const uint8_t COLOR_RED   = 1;
const uint8_t COLOR_GREEN = 2;

volatile char current_color;
volatile char is_blink;

inline void switch_color(char col) {
   setPortB(0b11000);
   if (col == COLOR_RED) 
     resetPortB(0b01000);
   if (col == COLOR_GREEN) 
     resetPortB(0b10000);
}

inline void setColor(const char col, const char blink) {
  current_color = col;
  is_blink = blink;
  //switch_color(col);
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
 * 	GetLight 0x01   Aktuellen Datenwert ausgeben
 *      SetLight 0x02   Neuen Datenwert setzen
 * 
 * data (DDDD)
 * 	1 bit blink-Status
 * 	3 bit Farbe:	0 keine
 * 			1 rot
 * 			2 grün
 */
#define CMD_I3C_RESET 0x00
#define CMD_GETLIGHT  0x01
#define CMD_SETLIGHT  0x02

static void twi_callback(uint8_t buffer_size,
                         volatile uint8_t input_buffer_length, 
                         volatile const uint8_t *input_buffer,
                         volatile uint8_t *output_buffer_length, 
                         volatile uint8_t *output_buffer) {
  
  if (input_buffer_length) {
    const char parity = (input_buffer[0] & 0x80) >> 7;
    const char cmd  = (input_buffer[0] & 0x70) >> 4;
    const char data = input_buffer[0] & 0x0F;
    
    // check parity
    char v = input_buffer[0] & 0x7F;
    char c;
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
	break;
      }
      case CMD_GETLIGHT: {
	const uint8_t state = ((is_blink == 0 ? 0 : 1) << 4) + current_color;
	output = state;

	// set msb to avoid error state
	output += 0x80;
	
	break;
      }
      case CMD_SETLIGHT: {
	setColor(data&0x7, data&0x8);
	output = 1;
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
   *   PB3: Rt (Out)
   *   PB4: Gn (Out)
   */
  DDRB  = 0b1111010;
  // PullUp für Eingänge
  PORTB = 0b11111010;

  /*
   * Set state: no color
   */
  setColor(COLOR_NONE, 0);

   
   /*  disable interrupts  */
   cli();
   
   
   /*  set clock   */
  CLKPR = (1 << CLKPCE);  /*  enable clock prescaler update       */
  CLKPR = 0;              /*  set clock to maximum                */

  /*  timer init  */
  TIFR &= ~(1 << TOV0);   /*  clear timer0 overflow interrupt flag    */
  TIMSK |= (1 << TOIE0);  /*  enable timer0 overflow interrupt        */

  /*  start timer0 by setting last 3 bits in timer0 control register B
   *  to any clock source */
  //TCCR0B = (1 << CS02) | (1 << CS00);
  TCCR0B = (1 << CS00);

    
  // Global Interrupts aktivieren
  sei();  
  
  i3c_tristate();
}

int main(void)
{
  // initialisieren
  init();

  // start TWI (I²C) slave mode
  usi_twi_slave(0x20, 0, &twi_callback, &twi_idle_callback);

  return 0;
}

const int TCOUNT_MAX = 20000;
volatile int tcount = 0;
volatile char blink = 0;

ISR (TIMER0_OVF_vect)
{
  // TODO be a bit more fancy at this point
  tcount++;
  if (tcount > TCOUNT_MAX) {
    tcount = 0;
    blink = !blink;
    switch_color(is_blink && !blink ? COLOR_NONE : current_color);
  }
}
