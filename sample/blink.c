//
// Blink on P51
//

#include "common.h"

void init(void) {
  // disable WDT
  init_watchdog(0);

  // enable GPIO
  // TRM Ch10 I/O Port - Table 2-2 I/O Port Functions and Register Setting Values
  FM3_GPIO->DDR5 = 0xFF;
}

void do_blink(void) {
  // NOTE: on 4MHz clock, delay(1000000) should wait ~1s
  FM3_GPIO->PDOR5_f.P1 = 0;
  delay(500000);
  FM3_GPIO->PDOR5_f.P1 = 1;
  delay(500000);
}

int main(void) {
  init();

  for (;;) {
    do_blink();
  }
}
