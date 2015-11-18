#include <Manchester.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#define TX_PIN 1  //pin where your transmitter is connected
#define LED_PIN 4 //pin for blinking LED

uint8_t moo = 1; //last led status
uint16_t transmit_data = 0;

// Routines to set and claer bits (used in the sleep code)
#ifndef cbi
  #define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
  #define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// Variables for the Sleep/power down modes:
volatile boolean f_wdt = 1;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, moo);
  man.setupTransmit(TX_PIN, MAN_4800);
  
      setup_watchdog(8); // approximately 0.5 seconds sleep
}

void loop() {
  static int counter =0;
    if (f_wdt==1) {  // wait for t
    f_wdt=0;       // reset flag
    while(counter<10) {
        transmit_data = analogRead(3);
      man.transmit(transmit_data);
      moo = ++moo % 2;
      digitalWrite(LED_PIN, moo);
      counter++;
    }
    counter =0;

      digitalWrite(LED_PIN, LOW);
        system_sleep();  // Send the unit to sleep
    }
}

// set system into the sleep state 
// system wakes up when wtchdog is timed out
void system_sleep() {
  
  cbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter OFF

  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
  sleep_enable();

  sleep_mode();                        // System actually sleeps here

  sleep_disable();                     // System continues execution here when watchdog timed out 
  
  sbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter ON
  
}

// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
void setup_watchdog(int ii) {

  byte bb;
  int ww;
  if (ii > 9 ) ii=9;
  bb=ii & 7;
  if (ii > 7) bb|= (1<<5);
  bb|= (1<<WDCE);
  ww=bb;

  MCUSR &= ~(1<<WDRF);
  // start timed sequence
  WDTCR |= (1<<WDCE) | (1<<WDE);
  // set new watchdog timeout value
  WDTCR = bb;
  WDTCR |= _BV(WDIE);
}
  
// Watchdog Interrupt Service / is executed when watchdog timed out
ISR(WDT_vect) {
  f_wdt=1;  // set global flag
}
