/*
 * BCD-Anzeige mit Schieberegister und PWM
 * Autor: Stefan Haun <tux@netz39.de>
 * 
 * Entwickelt für AT90S2343
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

inline void setPortB(char mask) {
  PORTB = PORTB | mask;
}

inline void resetPortB(char mask) {
  PORTB = PORTB & ~mask; 
}

const int COLOR_NONE  = 0;
const int COLOR_RED   = 1;
const int COLOR_GREEN = 2;

volatile char current_color;
volatile char is_blink;

inline void switch_color(char col) {
   setPortB(0b11);
   if (col == COLOR_RED) 
     resetPortB(0b01);
   if (col == COLOR_GREEN) 
     resetPortB(0b10);
}

inline void setColor(const char col, const char blink) {
  current_color = col;
  is_blink = blink;
  //switch_color(col);
}



void init(void) {
  /*
   * Pin-Config PortB:
   *   PB0: LED Rot (Out Invers)
   *   PB1: LED Grün (Out Invers)
   *   PB2: Set 1 (In)
   *   PB3: Set 2 (In)
   *   PB4: Set 3 (In)
   */
  DDRB  = 0b11100011;
  // PullUp für Eingänge
  /*
   * Aus bisher nicht geklärten Gründen ist der PullUp an PB4 nicht wirksam und
   * musste in der Schaltung ergänzt werden!
   */
  PORTB = 0b11111111;
   
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
  
  setColor(COLOR_NONE, 0);
}

/*
 */

int main(void)
{
  // initialisieren
  init();

  while(1) {
   // Pin-Belegung Port B auslesen
   char b = PINB;
   // e Pins in Hex-Ziffer umwandelns
   /*
    * Bei günstigerer PIN-Belegung lässt sich das auch als
    * int val = (b>>1)&0xF;
    * darstellen!
    */
   char red = !bit_is_set(b, PB2);
   char green = !bit_is_set(b, PB3);
   char blink = !bit_is_set(b, PB4);
   
   if (red)
     setColor(COLOR_RED, blink);
   else if (green)
     setColor(COLOR_GREEN, blink);
   else
     setColor(COLOR_NONE, blink);
   
  }
        
  return 0;
}

const int TCOUNT_MAX = 24;
volatile int tcount = 0;
volatile char blink = 0;
volatile char red = 0;

ISR (TIMER0_OVF_vect)
{
  if (++tcount > TCOUNT_MAX) {
    tcount = 0;
    blink = !blink;
    switch_color(is_blink && !blink ? COLOR_NONE : current_color);
  }
}
