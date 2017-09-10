/*
 * attiny2313a_7segx8_test
 *
 */
#include <msp430.h>
#include <signal.h>

#define kPeriod      12       // 12000 / 12 = 1000 [Hz]

#define kSlaveAddr 0x09

char buff[] = {
              0x3f, // 0
              0x06, // 1
              0x5b, // 2
              0x4f, // 3
              0x66, // 4
              0x6d, // 5
              0x7d, // 6
              0x07, // 7
              0x7f, // 8
              0x6f, // 9
              0x77, // a
              0x7c, // b
              0x39, // c
              0x5e, // d
              0x79, // e
              0x71, // f
              0x80, // dot
            };

void initialize_mpu(void)
{
  WDTCTL = WDTPW + WDTHOLD; // Stop watch dog

  if (CALBC1_1MHZ==0xFF) {  // If calibration constants erased
    while(1);               // do not load, trap CPU!!
  }
  DCOCTL = 0;              // Select lowest DCOx and MODx settings
  DCOCTL  = CALDCO_1MHZ;   // DCO pre setting 1/8/12/16MHz
  BCSCTL1 = CALBC1_1MHZ;   // DCO pre setting 1/8/12/16MHz

  // Setup Timer A
  TACTL = TASSEL_1 + ID_0 + MC_1;
  TACCR0 = kPeriod;
  TACCTL0 |= CCIE;              // enable interrupt

  BCSCTL3 |= LFXT1S_2;          // Select VLO for ACLK
}

void usi_i2c_initialize(void)
{
    P1OUT = 0xC0;                        // P1.6 & P1.7 Pullups, others to 0
    P1REN |= 0xC0;                       // P1.6 & P1.7 Pullups
    P1DIR = 0xFF;                        // Unused pins as outputs
    P2OUT = 0;
    P2DIR = 0xFF;

    USICTL0 = USIPE6+USIPE7+USIMST+USISWRST;    // Port & USI mode setup
    USICTL1 = USII2C;                           // Enable I2C mode
    USICKCTL = USIDIV_7+USISSEL_2+USICKPL;      // Setup USI clocks: SCL = SMCLK/128 (~7.8kHz)
    USICTL0 &= ~USISWRST;                       // Enable USI


}

#pragma vector = TIMERA0_VECTOR
__interrupt void interrupt_timer_A0(void)
{
  _BIC_SR_IRQ(SCG1 | SCG0 | CPUOFF);
}

void delay_ms(int ms_value)
{
  while(ms_value--){
    _BIS_SR(SCG1 | SCG0 | CPUOFF);
   };
}

void i2c_start_condition(void)
{
    while(!(P1IN & BIT6));
    USISRL = 0;
    USICTL0 |= USIGE + USIOE;
    USICTL0 &= ~USIGE;
}

void i2c_stop_condition(void)
{
  USICTL0 |= USIOE;
  USISRL = 0;
  USICNT = 1;
  while(!(USICTL1 & USIIFG));
  USISRL = 0xff;
  USICTL0 |= USIGE;
  USICTL0 &= ~(USIGE + USIOE);
}

/**
 * @brief  I2C transmission (1 byte)
 * @params aData the data byte
 * @return 1 if NACK, 0 if ACK
 */
int i2c_transmit(int aData)
{
  while(!(P1IN & BIT6));
  USISRL = aData;
  USICTL0 |= USIOE;
  USICNT = 8;
  while(!(USICTL1 & USIIFG));
  // receive ACK
  USICTL0 &= ~USIOE;
  USICNT = 1;
  while(!(USICTL1 & USIIFG));
  return USISRL & 1;
}


int main(void)
{
  int led;
  int buff_p;
  int i;

  initialize_mpu();
  usi_i2c_initialize();
  _BIS_SR(GIE);

  P1OUT &= ~BIT0;
  i2c_stop_condition();
  delay_ms(40);

  while(1) {

      // Test01
      for (led = 0; led < 8; led++) {

          for (buff_p = 0; buff_p < sizeof(buff); buff_p++) {
              i2c_start_condition();
              i2c_transmit(kSlaveAddr << 1);
              if(i2c_transmit(led)) P1OUT |= BIT0;
              if(i2c_transmit(buff[buff_p])) P1OUT |= BIT0;
              i2c_stop_condition();
              delay_ms(100);
          }
      }

      delay_ms(1000);

      // Test02
      for (led = 0; led < 8; led++) {
        buff_p = 0;
        i2c_start_condition();
        i2c_transmit(kSlaveAddr << 1);
        if(i2c_transmit(led)) P1OUT |= BIT0;
        for (i = led; i < 8; i++) {
            if(i2c_transmit(buff[buff_p])) P1OUT |= BIT0;
            buff_p++;
            if (buff_p >= sizeof(buff)) {
                buff_p = 0;
            }
        }
        i2c_stop_condition();
        delay_ms(500);
      }
      delay_ms(1000);

      // Test03
      led = 0;
      buff_p = 0;
      i2c_start_condition();
      i2c_transmit(kSlaveAddr << 1);
      if(i2c_transmit(led)) P1OUT |= BIT0;
      for (i = led; i < 8; i++) {
          if(i2c_transmit(buff[buff_p])) P1OUT |= BIT0;
          buff_p++;
          if (buff_p >= sizeof(buff)) {
              buff_p = 0;
          }
      }
      i2c_stop_condition();
      delay_ms(500);

      i2c_start_condition();
      i2c_transmit(kSlaveAddr << 1);
      if(i2c_transmit(0x15)) P1OUT |= BIT0;
      i2c_stop_condition();
      delay_ms(3000);

      i2c_start_condition();
      i2c_transmit(kSlaveAddr << 1);
      if(i2c_transmit(0x14)) P1OUT |= BIT0;
      i2c_stop_condition();
      delay_ms(3000);

      i2c_start_condition();
      i2c_transmit(kSlaveAddr << 1);
      if(i2c_transmit(0x13)) P1OUT |= BIT0;
      i2c_stop_condition();
      delay_ms(3000);

      i2c_start_condition();
      i2c_transmit(kSlaveAddr << 1);
      if(i2c_transmit(0x12)) P1OUT |= BIT0;
      i2c_stop_condition();
      delay_ms(3000);

      i2c_start_condition();
      i2c_transmit(kSlaveAddr << 1);
      if(i2c_transmit(0x11)) P1OUT |= BIT0;
      i2c_stop_condition();
      delay_ms(3000);

      i2c_start_condition();
      i2c_transmit(kSlaveAddr << 1);
      if(i2c_transmit(0x10)) P1OUT |= BIT0;
      i2c_stop_condition();
      delay_ms(3000);

      delay_ms(3000);

      // Test04
      i2c_start_condition();
      i2c_transmit(kSlaveAddr << 1);
      if(i2c_transmit(0x21)) P1OUT |= BIT0;
      i2c_stop_condition();
      delay_ms(3000);

      i2c_start_condition();
      i2c_transmit(kSlaveAddr << 1);
      if(i2c_transmit(0x20)) P1OUT |= BIT0;
      i2c_stop_condition();
      delay_ms(3000);


  }
}
