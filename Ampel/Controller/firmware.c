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

inline void setColor(const char col, const char blink) {
  current_color = col;
  is_blink = blink;
  switch_color(col);
}

inline void switch_color(const char col) {
   setPortB(0b11);
   if (col == COLOR_RED) 
     resetPortB(0b01);
   if (col == COLOR_GREEN) 
     resetPortB(0b10);
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
   
  /*
   * Timer-Code nach http://www.mikrocontroller.net/articles/AVR-GCC-Tutorial/Die_Timer_und_Z%C3%A4hler_des_AVR#8-Bit_Timer.2FCounter
   */
  
  // Timer 1 konfigurieren - FIXME korrekt setzen!
  TCCR1 = (1<<CS02)+(0<<CS01)+(0<<CS00); // Prescaler 256
 
  // Overflow Interrupt erlauben
  TIMSK |= (1<<TOIE0); 
  
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
   int val = bit_is_set(b, PB2) ? 0 : 1;
   val += bit_is_set(b, PB3) ? 0 : 2;
   val += bit_is_set(b, PB4) ? 0 : 4; 
    
   char red = !bit_is_set(b, PB2);
   char green = !bit_is_set(b, PB3);
   char blink = !bit_is_set(b, PB4);
   
   if (red)
     setColor(COLOR_RED, blink);
   else if (green)
     setColor(COLOR_GREEN, blink);
   else
     setColor(COLOR_NONE, 0);
   
  }
        
  return 0;
}

/*
 * Timer-Code nach http://www.mikrocontroller.net/articles/AVR-GCC-Tutorial/Die_Timer_und_Z%C3%A4hler_des_AVR#8-Bit_Timer.2FCounter
 */

/*
 * Der Overflow Interrupt Handler
 * wird aufgerufen, wenn TCNT0 von
 * 255 auf 0 wechselt (256 Schritte),
 * d.h. ca. alle 2 ms
 */
#ifndef TIMER0_OVF_vect
// Für ältere WinAVR Versionen z.B. WinAVR-20071221 
#define TIMER0_OVF_vect TIMER0_OVF0_vect
#endif
 
/* 
 * Interrupt-Aktion bei Timer-Überlauf
 */
ISR (TIMER1_OVF_vect)
{
  switch_color(current_color);
  // ggf. blinken
  
  // Counter hochsetzen, wir wollen keinen ganzen Zyklus warten
  TCNT1 = 0;
}
