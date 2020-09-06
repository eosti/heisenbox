/*
 * TM1637.h
 *
 */

#ifndef TM1637_H_
#define TM1637_H_

// Prototypes
static void TM1637_send_config(const int enable, const int brightness);
static void TM1637_send_command(const int value);
static void TM1637_start(void);
static void TM1637_stop(void);
static int TM1637_write_byte(int value);



#endif /* TM1637_H_ */

// Main Settings
#define TM1637_DIO_PIN          3 // Pin 1.3 and 1.4
#define TM1637_CLK_PIN          4
#define TM1637_DELAY_US         (5)
#define TM1637_BRIGHTNESS_MAX   (7)
#define TM1637_POSITION_MAX     (4)
