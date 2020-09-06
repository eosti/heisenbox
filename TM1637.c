/*
 * TM1627.c
 *
 * Provides driver support for the TM1627 display
 * Original by @lpodkalicki
 * https://github.com/lpodkalicki/attiny-tm1637-library
 * Adapted for MSP430 by @eosti
 *
 *
 */

#include "TM1637.h"
#include "msp430.h"

// TM1637 commands
#define TM1637_CMD_SET_DATA     0x40
#define TM1637_CMD_SET_ADDR     0xC0
#define TM1637_CMD_SET_DSIPLAY      0x80

// TM1637 data settings (use bitwise OR to construct complete command)
#define TM1637_SET_DATA_WRITE       0x00 // write data to the display register
#define TM1637_SET_DATA_READ        0x02 // read the key scan data
#define TM1637_SET_DATA_A_ADDR      0x00 // automatic address increment
#define TM1637_SET_DATA_F_ADDR      0x04 // fixed address
#define TM1637_SET_DATA_M_NORM      0x00 // normal mode
#define TM1637_SET_DATA_M_TEST      0x10 // test mode

// TM1637 display control command set (use bitwise OR to construct complete command)
#define TM1637_SET_DISPLAY_OFF      0x00 // off
#define TM1637_SET_DISPLAY_ON       0x08 // on


#define TM1637_DIO_HIGH()       (P1OUT |= _BV(TM1637_DIO_PIN))  // Sets data line high/low
#define TM1637_DIO_LOW()        (P1OUT &= ~_BV(TM1637_DIO_PIN))
#define TM1637_DIO_OUTPUT()     (P1DIR |= _BV(TM1637_DIO_PIN)) // Sets data line in/out
#define TM1637_DIO_INPUT()      (P1DIR &= ~_BV(TM1637_DIO_PIN))
#define TM1637_DIO_READ()       (((P1IN & _BV(TM1637_DIO_PIN)) > 0) ? 1 : 0) // Reads data line
#define TM1637_CLK_HIGH()       (P1OUT |= _BV(TM1637_CLK_PIN)) // Sets clock line high/low
#define TM1637_CLK_LOW()        (P1OUT &= ~_BV(TM1637_CLK_PIN))

 #define _BV(val) 1<<val // Bit shift macro

static int _config = TM1637_SET_DISPLAY_ON | TM1637_BRIGHTNESS_MAX;
static int _segments = 0xff;

static const int _digit2segments[] =
{
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F  // 9
};

void
_delay_us(const unsigned long us)
{
    unsigned long cycles = us * 16;
    while (0 < cycles)
    {
      __delay_cycles(1);
      --cycles;
    }
}

void
_delay_ms(const int ms)
{
    unsigned long us = ms * 1000;
    while (0 < us)
    {
        _delay_us(1);
        --us;
    }
}

void
TM1637_init(const int enable, const int brightness)
{

    P1SEL &= ~(_BV(TM1637_DIO_PIN)|_BV(TM1637_CLK_PIN)); // Sets pins as GPIO
    P1DIR |= (_BV(TM1637_DIO_PIN)|_BV(TM1637_CLK_PIN)); // Sets pins as outputs
    P1OUT &= ~(_BV(TM1637_DIO_PIN)|_BV(TM1637_CLK_PIN)); // Sets pins high (?)
    TM1637_send_config(enable, brightness);
}

void
TM1637_enable(const int value)
{

    TM1637_send_config(value, _config & TM1637_BRIGHTNESS_MAX);
}

void
TM1637_set_brightness(const int value)
{

    TM1637_send_config(_config & TM1637_SET_DISPLAY_ON,
        value & TM1637_BRIGHTNESS_MAX);
}

void
TM1637_display_segments(const int position, const int segments)
{

    TM1637_send_command(TM1637_CMD_SET_DATA | TM1637_SET_DATA_F_ADDR);
    TM1637_start();
    TM1637_write_byte(TM1637_CMD_SET_ADDR | (position & (TM1637_POSITION_MAX - 1)));
    TM1637_write_byte(segments);
    TM1637_stop();
}

void
TM1637_display_digit(const int position, const int digit)
{
    int segments = (digit < 10 ? _digit2segments[digit] : 0x00); // TODO wont work

    if (position == 0x01) {
        segments = segments | (_segments & 0x80);
        _segments = segments;
    }

    TM1637_display_segments(position, segments);
}

void
TM1637_display_colon(const int value)
{

    if (value) {
        _segments |= 0x80;
    } else {
        _segments &= ~0x80;
    }
    TM1637_display_segments(0x01, _segments);
}

void
TM1637_clear(void)
{
    int i;

    for (i = 0; i < TM1637_POSITION_MAX; ++i) {
        TM1637_display_segments(i, 0x00);
    }
}


void
TM1637_send_config(const int enable, const int brightness)
{

    _config = (enable ? TM1637_SET_DISPLAY_ON : TM1637_SET_DISPLAY_OFF) |
        (brightness > TM1637_BRIGHTNESS_MAX ? TM1637_BRIGHTNESS_MAX : brightness);

    TM1637_send_command(TM1637_CMD_SET_DSIPLAY | _config);
}

void
TM1637_send_command(const int value)
{

    TM1637_start();
    TM1637_write_byte(value);
    TM1637_stop();
}

void
TM1637_start(void)
{

    TM1637_DIO_HIGH();
    TM1637_CLK_HIGH();
    _delay_us(TM1637_DELAY_US);
    TM1637_DIO_LOW();
}

void
TM1637_stop(void)
{

    TM1637_CLK_LOW();
    _delay_us(TM1637_DELAY_US);

    TM1637_DIO_LOW();
    _delay_us(TM1637_DELAY_US);

    TM1637_CLK_HIGH();
    _delay_us(TM1637_DELAY_US);

    TM1637_DIO_HIGH();
}

int
TM1637_write_byte(int value)
{
    int i, ack;

    for (i = 0; i < 8; ++i, value >>= 1) {
        TM1637_CLK_LOW();
        _delay_us(TM1637_DELAY_US);

        if (value & 0x01) {
            TM1637_DIO_HIGH();
        } else {
            TM1637_DIO_LOW();
        }

        TM1637_CLK_HIGH();
        _delay_us(TM1637_DELAY_US);
    }

    TM1637_CLK_LOW();
    TM1637_DIO_INPUT();
    TM1637_DIO_HIGH();
    _delay_us(TM1637_DELAY_US);

    ack = TM1637_DIO_READ();
    if (ack) {
        TM1637_DIO_OUTPUT();
        TM1637_DIO_LOW();
    }
    _delay_us(TM1637_DELAY_US);

    TM1637_CLK_HIGH();
    _delay_us(TM1637_DELAY_US);

    TM1637_CLK_LOW();
    _delay_us(TM1637_DELAY_US);

    TM1637_DIO_OUTPUT();

    return ack;
}

