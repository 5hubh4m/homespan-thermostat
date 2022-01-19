#include <driver/adc.h>
#include <esp_adc_cal.h>

// this class incorporates the temperature sensor
class TemperatureSensor {
private:
  // characteristics for the ADC
  esp_adc_cal_characteristics_t adc1_chars;

  // ADC channel of the temperature sensor
  adc1_channel_t tempSensorChannel;

public:
  // construct and setup the ADC parameters for the TMP36 sensor
  TemperatureSensor(adc1_channel_t channel): tempSensorChannel(channel) {
    // set attentuation to 2.5 Db as it allows measurements
    // of upto 1050 mV on ESP32-S2 which is 50.5 °C for TMP36
    adc1_config_channel_atten(tempSensorChannel, ADC_ATTEN_DB_2_5);

    // set measurement width to maximum for highest precision
    adc1_config_width(ADC_WIDTH_BIT_13);

    // characterise the ADC to ustilise the calibration eFUSE
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_2_5, ADC_WIDTH_BIT_13, 0, &adc1_chars);
  }

  // read the temperature in decacelsius
  int readTemp() {
    // read the raw ADC value and convert it to mV
    int voltage = esp_adc_cal_raw_to_voltage(adc1_get_raw(tempSensorChannel), &adc1_chars);

    // convert to decacelsius (https://learn.adafruit.com/tmp36-temperature-sensor)
    return (voltage - 500);
  }
};
