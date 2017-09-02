/*
 * attiny2313a_7segx8_i2c_slave.h
 *
 * Created: 2017/08/27 1:33:47
 *  Author: Kenichi KAWABATA
 */ 


#ifndef ATTINY2313A_7SEGX8_I2C_SLAVE_H_
#define ATTINY2313A_7SEGX8_I2C_SLAVE_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

/******************************************************************************
 *       Micro Controller Dependent Definitions
 ******************************************************************************/
#if defined(__AVR_ATtiny2313A__)
#define DDR_USI         DDRB
#define PORT_USI        PORTB
#define PIN_USI         PINB
#define PORT_USI_SDA    PORTB5
#define PORT_USI_SCL    PORTB7
#define PIN_USI_SDA     PINB5
#define PIN_USI_SCL     PINB7
#else
#error Not Support!!
#endif


/******************************************************************************
 *       Functions Implemented as Macros
 ******************************************************************************/
#define SET_USI_TO_SEND_ACK()                                                           \
    {                                                                                   \
        USIDR = 0;                      /* Prepare ACK                              */  \
        DDR_USI |= (1 << PORT_USI_SDA); /* Set SDA as output                        */  \
        USISR = (0 << USISIF) |         /* Clear Start Condition Interrupt Flag     */  \
                (1 << USIOIF) |         /* Enable Counter Overflow Interrupt Flag   */  \
                (1 << USIPF)  |         /* Clear Stop Condition Flag                */  \
                (1 << USIDC)  |         /* Clear Data Output Collision Flag         */  \
                (0x0E << USICNT0);      /* Set USI Counter to shift 1 bit.          */  \
    }

#define SET_USI_TO_TWI_START_CONDITION_MODE()                                           \
    {                                                                                   \
        USICR = (1 << USISIE) |         /* Enable Start Condition Interrupt.        */  \
                (0 << USIOIE) |         /* Disable Overflow Interrupt.              */  \
                (1 << USIWM1) |         /* Set USI in Two-wire mode.                */  \
                (0 << USIWM0) |         /* No USI Counter overflow hold.            */  \
                (1 << USICS1) |         /* External Shift Register Clock            */  \
                (0 << USICS0) |         /* Positive Edge                            */  \
                (0 << USICLK) |         /* Both Edges                               */  \
                (0 << USITC);                                                           \
        USISR = (0 << USISIF) |         /* Clear Start Condition Interrupt Flag     */  \
                (1 << USIOIF) |         /* Enable Counter Overflow Interrupt Flag   */  \
                (1 << USIPF)  |         /* Clear Stop Condition Flag                */  \
                (1 << USIDC)  |         /* Clear Data Output Collision Flag         */  \
                (0x0 << USICNT0);       /* Set USI to shift out 8 bits              */  \
    }

#define SET_USI_TO_SEND_DATA()                                                          \
    {                                                                                   \
        DDR_USI |= (1 << PORT_USI_SDA); /* Set SDA as output                        */  \
        USISR = (0 << USISIF) |         /* Clear Start Condition Interrupt Flag     */  \
                (1 << USIOIF) |         /* Enable Counter Overflow Interrupt Flag   */  \
                (1 << USIPF)  |         /* Clear Stop Condition Flag                */  \
                (1 << USIDC)  |         /* Clear Data Output Collision Flag         */  \
                (0x0 << USICNT0);       /* Set USI to shift out 8 bits              */  \
    }

#define SET_USI_TO_READ_ACK()                                                           \
    {                                                                                   \
        DDR_USI &= ~(1 << PORT_USI_SDA);    /* Set SDA as input                     */  \
        USIDR = 0;                          /* Prepare ACK                          */  \
        USISR = (0 << USISIF) |         /* Clear Start Condition Interrupt Flag     */  \
                (1 << USIOIF) |         /* Enable Counter Overflow Interrupt Flag   */  \
                (1 << USIPF)  |         /* Clear Stop Condition Flag                */  \
                (1 << USIDC)  |         /* Clear Data Output Collision Flag         */  \
                (0x0E << USICNT0);      /* Set USI Counter to shift 1 bit.          */  \
    }

#define SET_USI_TO_READ_DATA()                                                          \
    {                                                                                   \
        DDR_USI &= ~(1 << PORT_USI_SDA);    /* Set SDA as input                     */  \
        USISR = (0 << USISIF) |         /* Clear Start Condition Interrupt Flag     */  \
                (1 << USIOIF) |         /* Enable Counter Overflow Interrupt Flag   */  \
                (1 << USIPF)  |         /* Clear Stop Condition Flag                */  \
                (1 << USIDC)  |         /* Clear Data Output Collision Flag         */  \
                (0x0 << USICNT0);       /* Set USI to shift out 8 bits              */  \
    }


/******************************************************************************
 *       USI Status
 ******************************************************************************/
#define USI_SLAVE_CHECK_ADDRESS                 (0x00)
#define USI_SLAVE_SEND_DATA                     (0x01)
#define USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA  (0x02)
#define USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA    (0x03)
#define USI_SLAVE_REQUEST_DATA                  (0x04)
#define USI_SLAVE_GET_DATA_AND_SEND_ACK         (0x05)


#endif /* ATTINY2313A_7SEGX8_I2C_SLAVE_H_ */