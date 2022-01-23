#include <driver/adc.h>
#include <esp_adc_cal.h>

// number of samples to average
#define SAMPLES 2

// this class incorporates the analog sensor
class AnalogSensor {
private:
  // characteristics for the ADC
  esp_adc_cal_characteristics_t adc1_chars;

  // ADC channel of the sensor
  adc1_channel_t channel;

public:
  // construct and setup the ADC parameters
  AnalogSensor(int pin) {
    // find the channel corresponding to the pin
    channel = (adc1_channel_t) (((int) ADC1_CHANNEL_2) + (pin - 3));

    // set attentuation to maximum (11 Db) as it allows measurements
    // of full range of the ADC (upto 3.3 V)
    adc1_config_channel_atten(channel, ADC_ATTEN_DB_11);

    // set measurement width to maximum for highest precision
    adc1_config_width(ADC_WIDTH_BIT_13);

    // characterise the ADC to utilise the calibration eFUSE
    memset(&adc1_chars, 0, sizeof(adc1_chars));
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_13, 0, &adc1_chars);
  }

public:
  // read the value in millivolts
  int readMilliVolts() {
    // multisampling for a better reading
    int raw = 0;
    for (int i = 0; i < SAMPLES; i++) {
        raw += adc1_get_raw(channel);
    }
    raw /= SAMPLES;

    // convert it to mV
    return esp_adc_cal_raw_to_voltage(raw, &adc1_chars);
  }
};

// this class incorporates a TMP36 temperature sensor
// https://learn.adafruit.com/tmp36-temperature-sensor
class TemperatureSensor {
private:
  // the ADC sensing object
  AnalogSensor sensor;

public:
  // constructor for the class
  TemperatureSensor(int pin): sensor(pin) {}

  // read the temperature in celsius
  float readTemp() {
    return (sensor.readMilliVolts() - 500) / 10;
  }
};
