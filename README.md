# 2 channel 12-bit I2C-controlled PWM generator based on Digispark with linear PWM level fading

I wrote this because i needed to control some LED lights with esp8266. But the esp's PWM is low resolution and since it's a software implementation i sometimes jitters causing attached lights to blink.\
Also, low resolution means that when slowly turning the light on or off, each brightness step is clearly visible - especially on darker settings.
With the help of ATTINY-based Digispark you can now have a fully hardware-based, high resolution PWM added to any microcontroller.
Since fading (up to a little over 1h long) is also done by the digispark, the main MCU can do better things or simnply go to sleep.

Sure, there are ready-made hardware solutions like PCA9685. but I needed something smaller. Plus, i had some Digisparks lying around :)

### Pinout:
* P0 - SDA (I2C data)
* P1 - PWM channel 0
* P2 - SCL (I2C clock)
* P4 - PWM channel 1

### I2C data frame:
* byte 0 - channel number (0 or 1)  
* byte 1 - target value (from 0 to 10000), upper byte  
* byte 2 - target value (from 0 to 10000), lower byte  
* byte 3 - transition time in ms (from 0 to 65535), upper byte  
* byte 4 - transition time in ms (from 0 to 65535), lower byte  
* byte 5 - gamma correction x 100 (from 0 to 255)  

### Using with Arduino Wire library (from another host):
    uint8_t  digispark_address = 0x42;
    uint8_t  channel = 0;
    uint16_t target_val = 5000; // half brightness
    uint16_t fade_time = 250;
    uint8_t  gamma = 180; //gamma correction factor 1.8

    Wire.beginTransmission(digispark_address);
    Wire.write(channel);
    Wire.write(target_val >> 8);
    Wire.write(target_val & 0xFF);
    Wire.write(fade_time >> 8);
    Wire.write(fade_time & 0xFF);
    Wire.write(gamma);
    Wire.endTransmission();


## Note: framework needs to be set to use TIMER0 for millis in core_build options, in __AVR_ATtinyX5__ section.
Default file location:  
  *%userprofile%\.platformio\packages\framework-arduinoavr@2.10623.190209\cores\digispark_tiny\core_build_options.h*
  
Find the line:

    #if defined( __AVR_ATtinyX5__ )
  
And change this line right below it:

    #define TIMER_TO_USE_FOR_MILLIS   1
    
To:

    #define TIMER_TO_USE_FOR_MILLIS   0
    

I havne't found a way to automatically change it without manually modifying framework headers, so compiling without the change will throw an error.
