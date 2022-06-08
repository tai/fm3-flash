
#include "common.h"

// Delay by a nop loop.
// It takes around 4[instructions/loop], so to wait 1s on 4MHz clock,
// delay(1000000) should do the job.
void delay(uint32_t loops) {
  while (loops--) {
    asm("nop");
  }
}

void init_watchdog(bool onoff) {
  while (FM3_HWWDT->WDG_LCK != 0) {
    FM3_HWWDT->WDG_LCK = 0x1ACCE551;
    FM3_HWWDT->WDG_LCK = 0xE5331AAE;
  }
  FM3_HWWDT->WDG_CTL = onoff;
}

//
// set master clock to PLL clock (~20MHz)
//
void init_clock(void) {
  //
  // Setup CLKHC (High-Speed CR clock)
  // TRM 2-2, 2 High-Speed CR Trimming Function Configuration and Block Diagram
  //

  // unlock MCR_FTRM
  FM3_CRTRIM->MCR_RLR = 0x1ACCE554;

  // Provide divided clock to MFT input capture ch3
  FM3_CRTRIM->MCR_PSR = 0b01;
  //                      ^^CSR 00=1/4, 01=1/8(*), 10=1/16, 11=1/32

  // Trim CLKHC
  // - TRM Ch1 System Overview, Table 1-1 Peripheral Address Map
  //
  // Base Timer is on APB1, using PCLK1. With following setup,
  // master clock can be derived from P22 toggle rate.
  //
  // - base clock = master clock / DIV1
  // - PCLK1 = base clock / DIV1
  // - Base Timer ch0 clock = PCLK1 / DIV128
  // - Base Timer ch0 reload counter = 128
  // - Toggle P22 on every BT ch0 interrupt
  //
  // For 4MHz CLKHC, P22 toggle rate should be
  //
  //   4000000 / 128 / 128 = 244.14[Hz]
  //
  // Sample Calibration Result
  // - 01100 01110 gave 233.57Hz (3.83MHz) (default MCR_FTRM value)
  // - 01100 11001 gave 244.10Hz (4.00MHz)
  // - 11111 11111 gave 504.03Hz (8.26MHz) 
  // - 11111 00000 gave 475.44Hz (7.79MHz)
  // - 00000 11111 gave  79.25Hz (1.30MHz)
  // - 00000 00000 gave  47.42Hz (0.78MHz)
  FM3_CRTRIM->MCR_FTRM = BITS(,,01100, 11001);
  //                            ^^^^^coarse adj
  //                                   ^^^^^finer adj

  // lock MCR_FTRM
  FM3_CRTRIM->MCR_RLR = 0;

  //
  // Setup various base/bus clocks
  //

  FM3_CRG->BSC_PSR   = 0x00; // base clock (FCLK, HCLK) = master clock / DIV
  FM3_CRG->APBC0_PSR = 0x00; // APB0 bus clock (PCLK0) = base clock / DIV
  FM3_CRG->APBC1_PSR = 0x80; // APB1 bus clock (PCLK1) = base clock / DIV
  FM3_CRG->APBC2_PSR = 0x80; // APB2 bus clock (PCLK2) = base clock / DIV
  FM3_CRG->SWC_PSR   = 0x80; // Software watchdog clock (SWDOGCLK) = PCLK0 / DIV

  //
  // Setup PLL and switch master clock to CLKPLL
  //

  // [PLL] set input and wait time
  FM3_CRG->PSW_TMR = 0b00010000;
  //                      ^PINC 1=PLL input is high-speed CR clock (CLKHC)
  //                       ^-
  //                        ^POWT 000=default stabilization wait time

  // [PLL] set parameters
  // TRM Ch2-1 Clock, Table 3-4 Example of PLL multiplication ratio settings for TYPE3/TYPE7 products
  // > The frequency of the PLLin multiplied by "MxN" becomes PLLout
  //
  // NOTE:
  // - (M, N, K) values in table 3-4 is 1 larger than actual PLL[MNK] values
  // - Need to align with baudrate setting in FM3_MFS0_UART->BGR
  FM3_CRG->PLL_CTL1 = 0x00;
  //                    ^PLLK 0=K is 1
  //                     ^PLLM 0=M is 1
  FM3_CRG->PLL_CTL2 = 4;
  //                  ^PLLN 4=N is 5

  // [PLL] enable PLL (master clock is still CLKHC)
  //
  // master clock is selected from below in SCM_CTL.RCS:
  // - 000=high-speed CR clock (CLKHC, 4MHz)
  // - 001=main clock (CLKMO, XTAL)
  // - 010=PLL output (CLKPLL)
  // - 100=low-speed CR clock (CLKLC, 100kHz)
  // - 101=sub clock (CLKSO, XTAL)
  //
  FM3_CRG->SCM_CTL = 0b00010000;
  //                   ^^^RCS 000=use high-speed CR clock as master clock
  //                      ^PLLE 1=enable PLL (CLKPLL)
  //                       ^SOSCE 0=no subclock (CLKSO)
  //                        ^-
  //                         ^MOSCE 0=no main clock (CLKMO)
  //                          ^-

  // [PLL] wait PLL to be stable
  while (! FM3_CRG->SCM_STR_f.PLRDY);

  // switch master clock to CLKPLL
  FM3_CRG->SCM_CTL = 0b01010000;
  //                   ^^^RCS 010=set main PLL clock as master clock
  //                      ^PLLE 1=enable PLL
  //                       ^SOSCE 0=no subclock (CLKSO)
  //                        ^-
  //                         ^MOSCE 0=no main clock (CLKMO)
  //                          ^-

  // wait until master clock switches
  while ((FM3_CRG->SCM_CTL & 0xE0) != (FM3_CRG->SCM_STR & 0xE0));
}

//
// set master clock to low-speed CR clock (~100kHz)
//
void init_clock_lc(void) {

  //
  // Slow down CLKHC (High-Speed CR clock)
  // TRM 2-2, 2 High-Speed CR Trimming Function Configuration and Block Diagram
  //

  // unlock MCR_FTRM
  FM3_CRTRIM->MCR_RLR = 0x1ACCE554;

  // Set CLKHC to lowest speed possible
  // - TRM Ch1 System Overview, Table 1-1 Peripheral Address Map
  FM3_CRTRIM->MCR_FTRM = BITS(,,00000, 00000);
  //                            ^^^^^coarse adj
  //                                   ^^^^^finer adj

  // lock MCR_FTRM
  FM3_CRTRIM->MCR_RLR = 0;

  // Setup various base/bus clocks
  FM3_CRG->BSC_PSR   = 0x00; // base clock (FCLK, HCLK) = master clock / DIV
  FM3_CRG->APBC0_PSR = 0x00; // APB0 bus clock (PCLK0) = base clock / DIV
  FM3_CRG->APBC1_PSR = 0x80; // APB1 bus clock (PCLK1) = base clock / DIV
  FM3_CRG->APBC2_PSR = 0x80; // APB2 bus clock (PCLK2) = base clock / DIV
  FM3_CRG->SWC_PSR   = 0x80; // Software watchdog clock (SWDOGCLK) = PCLK0 / DIV

  // switch master clock to CLKLC
  // TRM Ch2-1 Clock, 5.1. System Clock Mode Control Register (SCM_CTL)
  FM3_CRG->SCM_CTL = 0b10000000;
  //                   ^^^RCS 100=set low-speed CR clock as master clock
  //                      ^PLLE 0=disable PLL
  //                       ^SOSCE 0=no subclock (CLKSO)
  //                        ^-
  //                         ^MOSCE 0=no main clock (CLKMO)
  //                          ^-

  // wait until master clock switches
  while ((FM3_CRG->SCM_CTL & 0xE0) != (FM3_CRG->SCM_STR & 0xE0));
}





