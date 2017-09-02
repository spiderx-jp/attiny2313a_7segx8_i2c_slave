/*
 * attiny2313a_7segx8_i2c_slave.c
 *
 * Created: 2017/09/02 8:24:44
 * Author : Kenichi KAWABATA
 */ 

#include "attiny2313a_7segx8_i2c_slave.h"

#define USI_I2C_SLAVE_ADDRESS  0x09
#define TRUE                    (1)
#define FALSE                   (0)

/******************************************************************************
 * Static Variables
 ******************************************************************************/
uint8_t gDisplayDataBuf[] = {       // 7seg data register
    0b00111111,		// Col0
    0b00000110,		// Col1
    0b01011011,		// Col2
    0b01001111,		// Col3
    0b01100110,		// Col4
    0b01101101,		// Col5
    0b01111101,		// Col6
    0b10000111,		// Col7
    0b00000000,		// Col0-back
    0b00000000,		// Col1-back
    0b00000000,		// Col2-back
    0b00000000,		// Col3-back
    0b00000000,		// Col4-back
    0b00000000,		// Col5-back
    0b00000000,		// Col6-back
    0b00000000		// Col7-back
};

uint8_t gDisplayDimmer = 0x00;      // Display Dimmer

static volatile uint8_t     gUsiTwiOverflowState;
static uint8_t              gUsiTwiSlaveAddress;

/******************************************************************************
 * Initialize CPU
 ******************************************************************************/
void init_cpu(void)
{
    // Clock Prescale Register
    CLKPR = 0b10000000; // CLKPCE=enable,  CLKPS3,2,1,0=clear
    CLKPR = 0b00000011; // CLKPCE=disable, CLKPS3,2,1,0=0b0011(1/8)

    // Port Data Direction Register (7seg LED : Output)
    DDRA  |= (_BV(DDA0) | _BV(DDA1) | _BV(DDA2));
    DDRB  |= (_BV(DDB0) | _BV(DDB1) | _BV(DDB2) | _BV(DDB3) | _BV(DDB4)             | _BV(DDB6));
    DDRD  |= (_BV(DDD0) | _BV(DDD1) | _BV(DDD2) | _BV(DDD3) | _BV(DDD4) | _BV(DDD5) | _BV(DDD6));

    // Port Data Register (7seg LED row switch : Active-Low)
    PORTA |=  (_BV(PORTA0) | _BV(PORTA1) | _BV(PORTA2));
    PORTB |=  (_BV(PORTB0) | _BV(PORTB1) | _BV(PORTB2) | _BV(PORTB3) | _BV(PORTB4));

    // Port Data Register (7seg LED column switch : Active-High)
    PORTD &= ~(_BV(PORTD0) | _BV(PORTD1) | _BV(PORTD2) | _BV(PORTD3) | _BV(PORTD4) | _BV(PORTD5) | _BV(PORTD6));
    PORTB &= ~_BV(PORTB6);

    // Timer/Counter
    TCCR0A = _BV(WGM01);        // Timer/Counter0 Control Register A
    TCCR0B = _BV(CS00);         // Timer/Counter0 Control Register B
    TCNT0 = 0;                  // Timer/Counter0
    OCR0A = 233;                // Timer/Counter0 Output Compare A Register A
                                //  => 8MHz/CLKPS(8)/CS(1)/223=4.48kHz (LED refresh is 560Hz)
    OCR0B = 0;                  // Timer/Counter0 Output Compare B Register B
                                //  => don't use.
    TIMSK = _BV(OCIE0A);        // Timer/Counter Interrupt Mask Register
                                // => Output Compare Match A Interrupt Enable

    return;
}

/******************************************************************************
 * Initialize USI for TWI Slave mode.
 ******************************************************************************/
void init_usi_twi_slave_mode(uint8_t addr) {

    gUsiTwiSlaveAddress = addr; /* store our address so we know when being addressed by master */

    PORT_USI |= (1 << PORT_USI_SCL);    // Set SCL high
    PORT_USI |= (1 << PORT_USI_SDA);    // Set SDA high

    DDR_USI |= (1 << PORT_USI_SCL);     // Set SCL as output
    DDR_USI &= ~(1 << PORT_USI_SDA);    // Set SDA as input

    USICR = (1 << USISIE) | (0 << USIOIE) | // Enable Start Condition Interrupt. Disable Overflow Interrupt.
            (1 << USIWM1) | (0 << USIWM0) | // Set USI in Two-wire mode. No USI Counter overflow prior
                                            // to first Start Condition (potential failure)
            (1 << USICS1) | (0 << USICS0) | (0 << USICLK) | // Shift Register Clock Source = External, positive edge
            (0 << USITC);

    USISR = 0xF0; // Clear all flags and reset overflow counter

    return;
}

/******************************************************************************
 * Display 7segment x8
 ******************************************************************************/
void disp_7seg_x8(const uint8_t column_data, const uint8_t row_data)
{
    uint8_t     data_porta;     // Port A data
    uint8_t     data_portb;     // Port B data

    data_porta = (row_data >> 5);
    data_portb = (row_data & 0b00011111);

    // Column All Off
    PORTD &= ~(_BV(PD6) | _BV(PD5) | _BV(PD4) | _BV(PD3) | _BV(PD2) | _BV(PD1) | _BV(PD0));
    PORTB &= ~_BV(PB6);

    // Row All Off
    PORTA |= _BV(PA0) | _BV(PA1) | _BV(PA2);
    PORTB |= _BV(PB0) | _BV(PB1) | _BV(PB2) | _BV(PB3) | _BV(PB4);


    // Column Switch On
    switch(column_data)
    {
        case 0:
        PORTB |= _BV(PB6);
        break;

        case 1:
        PORTD |= _BV(PD6);
        break;
        
        case 2:
        PORTD |= _BV(PD5);
        break;

        case 3:
        PORTD |= _BV(PD4);
        break;

        case 4:
        PORTD |= _BV(PD3);
        break;

        case 5:
        PORTD |= _BV(PD2);
        break;

        case 6:
        PORTD |= _BV(PD1);
        break;

        default:            // led == 7
        PORTD |= _BV(PD0);
        break;
    }

    // Set Row data
    PORTA &= ~data_porta;
    PORTB &= ~data_portb;

    return;
}

/******************************************************************************
 * Refresh 7segment x8 display
 ******************************************************************************/
void reflesh_disp(void)
{
    static uint8_t      count1 = 0;             // Display refresh counter [0..255]
    static uint8_t      count2 = 0;             // Low bright counter [0..255]
    static uint8_t      *ptr = gDisplayDataBuf; // Data register pointer

    uint8_t             col;                    // Col Data
    uint8_t             row;                    // Row Data

    if((gDisplayDimmer != 0) && (count1 & gDisplayDimmer))
    {
        row = 0x00;
    }
    else
    {
        row = *ptr;
        if((count2 & 0x07) == 0)
        {
            row |= *(ptr + 8);
        }
    }
    
    col = (count1 & 0x07);
    disp_7seg_x8(col, row);

    count1++;
    ptr++;

    if((count1 & 0x07) == 0)
    {
        ptr = gDisplayDataBuf;
        count2++;
    }

    return;
}




/******************************************************************************
 * Detects the USI_TWI Start Condition and initialize the USI
 * for reception of the "TWI Address" packet.
 ******************************************************************************/
ISR(USI_START_vect) {
    uint8_t tmp_usisr;      // Temporary variable to store volatile
    tmp_usisr = USISR;      // Not necessary, but prevents warnings
                            // Set default starting conditions for new TWI package

    gUsiTwiOverflowState = USI_SLAVE_CHECK_ADDRESS;
    DDR_USI &= ~(1 << PORT_USI_SDA);    // Set SDA as input

    while ((PIN_USI & (1 << PORT_USI_SCL)) && !(tmp_usisr & (1 << USIPF)))
        ;   // Wait for SCL to go low to ensure the "Start Condition" has completed.
            // If a Stop condition arises then leave the interrupt to prevent waiting forever.
            //  SCL is High Level[(PIN_USI & (1 << PORT_USI_SCL))]
            //  And SDA is Low Level[!(tmp_usisr & (1 << USIPF))]

    USICR = (1 << USISIE) | // Keep START interrupt enabled
            (1 << USIOIE) | // Keep overflow interrupt enabled
            (1 << USIWM1) | // Two wire mode
            (1 << USIWM0) | // Two wire mode, this time with overflow clock held low
            (1 << USICS1) | // External positive edge shift register clock source
            (0 << USICS0) | // External both edges counter clock source
            (0 << USICLK) |
            (0 << USITC);   // don't toggle clock pin

    USISR = (1 << USISIF) | // Start condition detected
            (1 << USIOIF) | // Clear counter overflow interrupt flag
            (1 << USIPF)  | // Clear STOP condition flag
            (1 << USIDC)  | // Clear collision bit
            (0 << USICNT0); // Set USI to sample 8 bits i.e. count 16 external pin toggles.

}

/******************************************************************************
 * Handles all the communication. Is disabled only when waiting
 * for new Start Condition.
 ******************************************************************************/
ISR(USI_OVERFLOW_vect) {
    static uint8_t is_data0 = FALSE;
    static uint8_t disp_dp = 0;

    uint8_t tmp_usidr;

    switch (gUsiTwiOverflowState) {
        case USI_SLAVE_CHECK_ADDRESS:
            // ---------- Address mode ----------
            // Check address and send ACK (and next USI_SLAVE_SEND_DATA) if OK, else reset USI.
            if ((USIDR == 0) || ((USIDR >> 1) == gUsiTwiSlaveAddress)) {
                if (USIDR & 0x01) {
                    // if the read bit is set then we are sending data
                    gUsiTwiOverflowState = USI_SLAVE_SEND_DATA;
                } else {
                    // otherwise we will be receiving data
                    gUsiTwiOverflowState = USI_SLAVE_REQUEST_DATA;
                }
                SET_USI_TO_SEND_ACK();
            } else {
                SET_USI_TO_TWI_START_CONDITION_MODE();
            }
            is_data0 = TRUE;
            break;

        case USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA:
            // ----- Master write data mode ------
            // Check reply and goto USI_SLAVE_SEND_DATA if OK, else reset USI.
            if (USIDR) {
                SET_USI_TO_TWI_START_CONDITION_MODE();
                return;
            }
            // From here we just drop straight into USI_SLAVE_SEND_DATA if the master sent an ACK

        case USI_SLAVE_SEND_DATA:
            // Copy data from buffer to USIDR and set USI to shift byte.
            // Next USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA

            if (is_data0) {
                is_data0 = FALSE;
                USIDR = 0x00; // Dummy Data Set
                gUsiTwiOverflowState = USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA;
                SET_USI_TO_SEND_DATA();

            } else {
                SET_USI_TO_TWI_START_CONDITION_MODE();
            }
            break;

        case USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA:
            // Set USI to sample reply from master.
            // Next USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA
            gUsiTwiOverflowState = USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA;
            SET_USI_TO_READ_ACK();
            break;


        case USI_SLAVE_REQUEST_DATA:
            // ----- Master read data mode ------
            // Set USI to sample data from master.
            // Next USI_SLAVE_GET_DATA_AND_SEND_ACK.
            gUsiTwiOverflowState = USI_SLAVE_GET_DATA_AND_SEND_ACK;
            SET_USI_TO_READ_DATA();

            break;

        case USI_SLAVE_GET_DATA_AND_SEND_ACK:
            // Copy data from USIDR and send ACK.
            // Next USI_SLAVE_REQUEST_DATA
            tmp_usidr               = USIDR;    // Not necessary, but prevents warnings

            if (is_data0) {
                uint8_t cmd;
                uint8_t prm;

                is_data0 = FALSE;
                cmd = tmp_usidr & 0b11110000;
                prm = tmp_usidr & 0b00001111;
                switch (cmd) {
                    case 0x00:      // Set Address Command
                        disp_dp = prm;
                        break;

                    case 0x10:      // Set Dimmer Command
                        switch (prm) {
                            case 0: gDisplayDimmer = 0x00;   break;
                            case 1: gDisplayDimmer = 0x08;   break;
                            case 2: gDisplayDimmer = 0x18;   break;
                            case 3: gDisplayDimmer = 0x38;   break;
                            case 4: gDisplayDimmer = 0x78;   break;
                            case 5: gDisplayDimmer = 0xf8;   break;
                            default:                        break;
                        }
                        break;

                    case 0x20:
                        if (prm) {
                            TIMSK &= ~_BV(OCIE0A);	// Interrupt Disable
                        } else {
                            TIMSK |= _BV(OCIE0A);	// Interrupt Enable
                        }
                        break;
                    default:
                        break;
                }
            } else {
                gDisplayDataBuf[disp_dp] = tmp_usidr;   // Set Data
                disp_dp = (disp_dp + 1) & 7;

            }

            gUsiTwiOverflowState = USI_SLAVE_REQUEST_DATA;
            SET_USI_TO_SEND_ACK();
            break;
        
        default:
            break;
    }
}


ISR(TIMER0_COMPA_vect)
{
    reflesh_disp();
    return;
}

int main(void)
{
    init_cpu();
    init_usi_twi_slave_mode(USI_I2C_SLAVE_ADDRESS);  // 

    set_sleep_mode(SLEEP_MODE_IDLE);
    sei();

    while (1) {
        sleep_mode();
    }

    return 0;
}

