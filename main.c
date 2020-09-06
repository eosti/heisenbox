#include <msp430.h> 
#include "TM1637.h"

// Motor defines
#define MOTOR_POS 3 //pin 2.3, 2.4, and 2.7
#define MOTOR_NEG 4
#define MOTOR_LIGHT 7

// Switch defines
#define REED_SWITCH 0 //pin 2.0

#define _BV(val) 1<<val // Bit shift macro

// Change TM1637 configuration within the TM1637.h file.

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

        // Set up reed switch
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

	        if (sleepCount > 650) { // Around 1 min
	            P2OUT &= ~(_BV(MOTOR_POS)); // Everything off...
	            P2OUT &= ~(_BV(MOTOR_NEG));
	            P2OUT &= ~(_BV(MOTOR_LIGHT));
	            TM1637_enable(0);

	            __bis_SR_register(LPM4_bits); // Enter LPM4.
	            // Only way to return is to reset.
	        }

	    }
}
