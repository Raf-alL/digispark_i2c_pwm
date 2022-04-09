#ifndef MAIN_H
#define MAIN_H

//#######################################################################################################
//####################### Digispark 2 channel 12-bit I2C-controlled PWM generator #######################
//#######################################################################################################

// Pinout:
//  P0 - SDA (I2C data)
//  P1 - PWM channel 0
//  P2 - SCL (I2C clock)
//  P4 - PWM channel 1

// I2C data frame:
//  byte 0 - channel number (0 or 1)
//  byte 1 - target value (from 0 to 10000), upper byte
//  byte 2 - target value (from 0 to 10000), lower byte
//  byte 3 - transition time in ms (from 0 to 65535), upper byte
//  byte 4 - transition time in ms (from 0 to 65535), lower byte
//  byte 5 - gamma correction x 100 (from 0 to 255)

// Using with Arduino Wire library (from another host):
//  uint8_t  digispark_address = 0x42;
//  uint8_t  channel = 0;
//  uint16_t target_val = 5000; // half brightness
//  uint16_t fade_time = 250;
//  uint8_t  gamma = 180; //gamma correction factor 1.8
// 
//  Wire.beginTransmission(digispark_address);
//  Wire.write(channel);
//  Wire.write(target_val >> 8);
//  Wire.write(target_val & 0xFF);
//  Wire.write(fade_time >> 8);
//  Wire.write(fade_time & 0xFF);
//  Wire.write(gamma);
//  Wire.endTransmission();

//##################################### [SETTINGS] ######################################
#define PWM_RESOLUTION 12       //set to 10 or 12 bits
#define I2C_SLAVE_ADDRESS 0x42  //I2C address to use (only lower 7 bits are alllowed, so from 0 to 127)
//##################################### [/SETTINGS] #####################################



#include <Arduino.h>
#include <TinyWireS.h>



#if PWM_RESOLUTION == 10
 #define PWM_CYCLE_MASK 0x03
 #define PWM_MAX_VALUE 1023  
#elif PWM_RESOLUTION == 12
 #define PWM_CYCLE_MASK 0x0F
 #define PWM_MAX_VALUE 4095 
#else
 #error "PWM_RESOLUTION MUST BE DEFINED AS 10 or 12 (in bits)"
#endif
#if TIMER_TO_USE_FOR_MILLIS == 1
 #error "TIMER_TO_USE_FOR_MILLIS must me set to 0 in core_build_options.h"
#endif
// Note: framework needs to be set to use TIMER0 for millis in core_build options, in __AVR_ATtinyX5__ section.
// Default file location:
//    %userprofile%\.platformio\packages\framework-arduinoavr@2.10623.190209\cores\digispark_tiny\core_build_options.h

volatile int Dac[] = {0,0};
volatile uint8_t* Port[] = { &OCR1A, &OCR1B };
volatile int Cycle = 0;

float pwm_start_val[2] = {0,0};
float pwm_current_val[2] = {0,0};
float pwm_target_val[2] = {0,0};
unsigned long pwm_fade_start_time[2] = {0,0};
uint16_t pwm_fade_time[2] = {0,0};
float pwm_gamma[2] = {2,2};



#ifndef TWI_RX_BUFFER_SIZE
#define TWI_RX_BUFFER_SIZE ( 16 )
#endif


uint8_t i2c_rx_buffer[10];
uint8_t i2c_rx_buffer_pos = 0;
unsigned long last_i2c_rx_time = 0;
const unsigned long i2c_rx_timeout = 50;

void i2cRequestEvent();
void i2cReceiveEvent(uint8_t howMany);

void analogWriteHiRes (uint8_t chnnel, int value);

void setup();

uint16_t gamma(uint16_t input, float gamma=2.0);

void loop ();





#endif //MAIN_H