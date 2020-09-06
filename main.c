#include <msp430.h> 

// Motor defines
#define MOTOR_POS 3 //pin 2.3, 2.4, and 2.7
#define MOTOR_NEG 4
#define MOTOR_LIGHT 7

// Switch defines
#define REED_SWITCH 0 //pin 2.0

// Main Settings
#define TM1637_DIO_PIN          3 // Pin 1.3 and 1.4
#define TM1637_CLK_PIN          4
#define TM1637_DELAY_US         (5)
#define TM1637_BRIGHTNESS_MAX   (7)
#define TM1637_POSITION_MAX     (4)

// TM1637 commands
#define TM1637_CMD_SET_DATA     0x40
#define TM1637_CMD_SET_ADDR     0xC0
#define TM1637_CMD_SET_DSIPLAY      0x80

// TM1637 data settings (use bitwise OR to contruct complete command)
#define TM1637_SET_DATA_WRITE       0x00 // write data to the display register
#define TM1637_SET_DATA_READ        0x02 // read the key scan data
#define TM1637_SET_DATA_A_ADDR      0x00 // automatic address increment
#define TM1637_SET_DATA_F_ADDR      0x04 // fixed address
#define TM1637_SET_DATA_M_NORM      0x00 // normal mode
#define TM1637_SET_DATA_M_TEST      0x10 // test mode

// TM1637 display control command set (use bitwise OR to consruct complete command)
#define TM1637_SET_DISPLAY_OFF      0x00 // off
#define TM1637_SET_DISPLAY_ON       0x08 // on

 #define _BV(val) 1<<val // Bit shift macro

#define TM1637_DIO_HIGH()       (P1OUT |= _BV(TM1637_DIO_PIN))  // Sets data line high/low
#define TM1637_DIO_LOW()        (P1OUT &= ~_BV(TM1637_DIO_PIN))
#define TM1637_DIO_OUTPUT()     (P1DIR |= _BV(TM1637_DIO_PIN)) // Sets data line in/out
#define TM1637_DIO_INPUT()      (P1DIR &= ~_BV(TM1637_DIO_PIN))
#define TM1637_DIO_READ()       (((P1IN & _BV(TM1637_DIO_PIN)) > 0) ? 1 : 0) // Reads data line
#define TM1637_CLK_HIGH()       (P1OUT |= _BV(TM1637_CLK_PIN)) // Sets clock line high/low
#define TM1637_CLK_LOW()        (P1OUT &= ~_BV(TM1637_CLK_PIN))

// Prototypes
static void TM1637_send_config(const int enable, const int brightness);
static void TM1637_send_command(const int value);
static void TM1637_start(void);
static void TM1637_stop(void);
static int TM1637_write_byte(int value);

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

/**
 * main.c
 */

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	    /* setup */
	    TM1637_init(1/*enable*/, 7/*brightness*/);

	    // Motor pins
	    P2SEL &= ~(_BV(MOTOR_POS)|_BV(MOTOR_NEG)|_BV(MOTOR_LIGHT)); // GPIOs, outs, off
	    P2DIR |= (_BV(MOTOR_POS)|_BV(MOTOR_NEG)|_BV(MOTOR_LIGHT));
	    P2OUT &= ~(_BV(MOTOR_POS)|_BV(MOTOR_NEG|_BV(MOTOR_LIGHT)));

        // Switch
	    P2SEL &= ~(_BV(REED_SWITCH)); // GPIO, ins, pullup
	    P2DIR &= ~(_BV(REED_SWITCH));
	    P2REN |= (_BV(REED_SWITCH));
	    P2OUT |= (_BV(REED_SWITCH));


	    /* loop */
	    unsigned long count = 9;
	    volatile int state;
	    volatile int direction = 0;
	    unsigned long sleepCount = 0;

	    TM1637_display_digit(1,4);  // Set default display as 1438
	    TM1637_display_digit(2,3);
	    TM1637_display_digit(3,8);
	    TM1637_display_digit(4,1);

	    while (1) {
	        state = (((P2IN & _BV(REED_SWITCH)) > 0) ? 1 : 0); // 1 is open, 0 closed
	        count++;
	        sleepCount++;

	        switch (state) {
	            case 0: // Case box closed
	                P2OUT &= ~(_BV(MOTOR_POS)|_BV(MOTOR_NEG)|_BV(MOTOR_LIGHT)); // Turns motors off
	                TM1637_enable(1);

	                if (count > 1) {
	                    TM1637_display_colon(0);
	                    TM1637_display_digit(4,1); // Set display as 1xxx
	                    if (sleepCount % 29 == 0) {
                            TM1637_display_digit(1, sleepCount * 7 % 10);
	                    }
	                    if (sleepCount % 7 == 0) {
	                        TM1637_display_digit(2, sleepCount % 10);
	                    }
	                    TM1637_display_digit(3, sleepCount * 13 % 10);
	                }
	                break;

	            case 1: //Case box open

	                if (count > 1) {
	                    TM1637_enable(0); // Turn off display
	                    P2OUT |= (_BV(MOTOR_LIGHT));

	                    if(direction == 0) { // If motor going forwards...
	                        P2OUT &= ~(_BV(MOTOR_POS)); //stop and then backwards
	                        P2OUT |= (_BV(MOTOR_NEG));
	                        direction = 1;
	                    } else { // Motor is going backwards
	                        P2OUT &= ~(_BV(MOTOR_NEG)); //stop and then backwards
	                        P2OUT |= (_BV(MOTOR_POS));
	                        direction = 0;
	                    }
	                    _delay_ms(1);
	                    count = 0;
	                }
	        }

	        if (sleepCount > 650) {
	            P2OUT &= ~(_BV(MOTOR_POS)); // Everything off...
	            P2OUT &= ~(_BV(MOTOR_NEG));
	            P2OUT &= ~(_BV(MOTOR_LIGHT));
	            TM1637_enable(0);

	            __bis_SR_register(LPM4_bits); // Enter LPM4.
	            // Only way to return is to reset.
	        }

	    }
}
