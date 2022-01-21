#include <driver/adc.h>
#include <esp_adc_cal.h>

// number of samples to average
#define SAMPLES 10

// this class incorporates the temperature sensor
class TemperatureSensor {
private:
  // characteristics for the ADC
  esp_adc_cal_characteristics_t adc1_chars;

  // ADC channel of the temperature sensor
  adc1_channel_t tempChannel;

public:
  // construct and setup the ADC parameters for the TMP36 sensor
  TemperatureSensor(int pin) {
    // find the channel corresponding to the pin
    tempChannel = (adc1_channel_t) (((int) ADC1_CHANNEL_2) + (pin - 3));

    // set attentuation to maximum (11 Db) as it allows measurements
    // of full range of the ADC (upto 3.3 V)
    adc1_config_channel_atten(tempChannel, ADC_ATTEN_DB_11);

    // set measurement width to maximum for highest precision
    adc1_config_width(ADC_WIDTH_BIT_13);

    // characterise the ADC to ustilise the calibration eFUSE
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_13, 0, &adc1_chars);
  }

  // read the temperature in decacelsius
  int readTemp() {
    // multisampling for a better reading
    int raw = 0;
    for (int i = 0; i < SAMPLES; i++) {
        raw += adc1_get_raw(tempChannel);
    }
    raw /= SAMPLES;

    // read the raw ADC value and convert it to mV
    int voltage = esp_adc_cal_raw_to_voltage(raw, &adc1_chars);

    // convert to decacelsius (https://learn.adafruit.com/tmp36-temperature-sensor)
    return (voltage - 500);
  }
};
