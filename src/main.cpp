#include "main.h"

void analogWriteHiRes(uint8_t channel, int value)
{
  cli();
  if (value < 0)
    value = 0;
  if (value > PWM_MAX_VALUE)
    value = PWM_MAX_VALUE;

  Dac[channel % 2] = value;
  sei();
}

void setup()
{
  // Top value for high (Table 12-2)
  OCR1C = 255;
  // Timer/Counter1 doing PWM on OC1A (PB1)
  TCCR1 = 1 << PWM1A    // Pulse Width Modulator A Enable
          | 1 << COM1A0 // OC1x cleared on compare match. Set when TCNT1 = $00
          | 1 << CS10;  // PWM clock = CK
  GTCCR = 1 << PWM1B | 1 << COM1B0;
  TIMSK |= 1 << TOIE1; // Timer/Counter1 Overflow Interrupt Enable
  pinMode(1, OUTPUT);
  pinMode(4, OUTPUT);

  TinyWireS.begin(I2C_SLAVE_ADDRESS);
  TinyWireS.onReceive(i2cReceiveEvent);
  TinyWireS.onRequest(i2cRequestEvent);
}

uint16_t gamma(uint16_t input, float gamma)
{
  return (uint16_t)(pow((float)input / (float)PWM_MAX_VALUE, gamma) * PWM_MAX_VALUE + 0.5);
}

void loop()
{
  TinyWireS_stop_check();
  for(uint8_t i=0; i<2; i++)
  {
    if(pwm_fade_time[i] == 0)
    {
      pwm_current_val[i] = pwm_target_val[i];  
    }
    else if(millis() - pwm_fade_start_time[i] > pwm_fade_time[i])
    {
      pwm_current_val[i] = pwm_target_val[i];
      pwm_fade_time[i] = 0;  
    }
    else
    {
      float progress = (millis() - pwm_fade_start_time[i]) / (float)pwm_fade_time[i];
      pwm_current_val[i] = pwm_start_val[i] + (pwm_target_val[i] - pwm_start_val[i]) * progress;
      if(pwm_current_val[i] < 0)
        pwm_current_val[i] = 0;
      else if(pwm_current_val[i] > 1)
        pwm_current_val[i] = 1;
    }
  

    int pwm_hw_val = pow( pwm_current_val[i], pwm_gamma[i])*PWM_MAX_VALUE;

    if(Dac[i] != pwm_hw_val)
      analogWriteHiRes(i, pwm_hw_val);

  }
  

  if(i2c_rx_buffer_pos > 0 && millis() - last_i2c_rx_time > i2c_rx_timeout)
  {
    for(uint8_t i=0; i< sizeof(i2c_rx_buffer); i++)
      i2c_rx_buffer[i] = 0;
    i2c_rx_buffer_pos = 0;
  }
}

ISR(TIMER1_OVF_vect)
{
  static int rem[2];
  for (int chan = 0; chan < 2; chan++)
  {
    int remain;
    if (Cycle == 0)
      remain = Dac[chan];
    else
      remain = rem[chan];
    if (remain >= 256)
    {
      *Port[chan] = 255;
      remain = remain - 256;
    }
    else
    {
      *Port[chan] = remain;
      remain = 0;
    }
    rem[chan] = remain;
  }
  Cycle = (Cycle + 1) & PWM_CYCLE_MASK;
}

void i2cRequestEvent()
{
  TinyWireS.send(0);
}
/**
 * The I2C data received -handler
 *
 * This needs to complete before the next incoming transaction (start, data, restart/stop) on the bus does
 * so be quick, set flags for long running tasks to be called from the mainloop instead of running them directly,
 */
void i2cReceiveEvent(uint8_t howMany)
{
  if (howMany < 1)
  {
    // Sanity-check
    return;
  }
  if (howMany > TWI_RX_BUFFER_SIZE)
  {
    // Also insane number
    return;
  }

  last_i2c_rx_time = millis();

  while (howMany--)
  {
    i2c_rx_buffer[i2c_rx_buffer_pos] = TinyWireS.receive();
    i2c_rx_buffer_pos++;
    if (i2c_rx_buffer_pos == 6)
    {
      if (i2c_rx_buffer[0] == 0 || i2c_rx_buffer[0] == 1) //valid channel
      {
        int target_val_x10000 = (i2c_rx_buffer[1] << 8 | i2c_rx_buffer[2]);
        if(target_val_x10000 >= 0 && target_val_x10000 <= 10000)
        {
          uint16_t fade_time = (i2c_rx_buffer[3] << 8 | i2c_rx_buffer[4]);
          uint8_t gamma_x100 = i2c_rx_buffer[5];
          
          pwm_start_val[i2c_rx_buffer[0]] = pwm_current_val[i2c_rx_buffer[0]];
          pwm_target_val[i2c_rx_buffer[0]] = target_val_x10000 / 10000.;
          pwm_fade_time[i2c_rx_buffer[0]] = fade_time;
          pwm_fade_start_time[i2c_rx_buffer[0]] = millis();
          pwm_gamma[i2c_rx_buffer[0]] = gamma_x100 / 100.;
        }
        else
        {
          
        }
      }
      i2c_rx_buffer_pos = 0;
    }
  }
}