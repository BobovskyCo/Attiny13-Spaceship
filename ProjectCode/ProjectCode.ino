#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>


#define LED_PIN 0     // PB0
#define LDR_PIN 3     // PB3 (ADC3)

volatile byte wdtCounter = 0;
volatile byte wdtWakeupsNeeded = 1; // сколько WDT сработок спим

// ================= WDT INTERRUPT =================
ISR(WDT_vect) {
  wdtCounter++;
}

// ================= SETUP WDT =================
void setupWDT_4s() {
  cli();
  wdt_reset();
  WDTCR |= (1 << WDCE) | (1 << WDE);   // разрешаем смену
  WDTCR = (1 << 6) | (1 << WDP3) | (1 << WDP0);    // 8 секунд
  sei();
}

// ================= SLEEP =================
void goSleep() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_cpu();
  sleep_disable();
}

// ================= LED =================
void blink(byte times) {
  for (byte i = 0; i < times; i++) {
    PORTB |= (1 << LED_PIN);
    delay(50);
    PORTB &= ~(1 << LED_PIN);
    delay(100);
  }
}

// ================= ADC READ =================
uint16_t readLight() {
  ADMUX = (1 << MUX1) | (1 << MUX0); // ADC3
  ADCSRA |= (1 << ADEN);            // включаем ADC
  ADCSRA |= (1 << ADSC);            // старт измерения
  while (ADCSRA & (1 << ADSC));     // ждём
  uint16_t value = ADC;
  ADCSRA &= ~(1 << ADEN);           // выключаем ADC
  return value;
}

// ================= SETUP =================
void setup() {
  DDRB |= (1 << LED_PIN);   // LED выход
  PORTB &= ~(1 << LED_PIN);
  setupWDT_4s();
}

// ================= LOOP =================
void loop() {

  wdtCounter = 0;

  uint16_t light = readLight();

  if (light > 300) {      // светло
    blink(3);
    wdtWakeupsNeeded = 1; // 4 сек
  } else {                // темно
    blink(1);
    wdtWakeupsNeeded = 4; // 16 сек (4 + 4 + 4 + 4)
  }

  while (wdtCounter < wdtWakeupsNeeded) {
    goSleep();
  }
}