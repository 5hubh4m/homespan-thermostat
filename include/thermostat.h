#include <HomeSpan.h>

#include "relay.h"
#include "neopixel.h"
#include "temperature.h"

// the class that contains the logic for running the
// thermostat service
class Thermostat: public Service::Thermostat {
private:
  // setup the sensor, heater relay, and status LED
  TemperatureSensor tempSensor;
  NeoPixel status;
  Relay heater;

  // setup the characteristic objects
  SpanCharacteristic *name;
  SpanCharacteristic *temperatureDisplayUnits;
  SpanCharacteristic *currentTemperature;
  SpanCharacteristic *currentHeaterState;
  SpanCharacteristic *targetHeaterState;
  SpanCharacteristic *targetTemperature;
  SpanCharacteristic *heatingThresholdTemperature;
  SpanCharacteristic *coolingThresholdTemperature;

  // track the state of the thermostat
  unsigned long lastSenseTemperature;
  unsigned long lastUpdateState;
  unsigned int tempReadings;
  long tempSum;

public:
  // constructor for the thermostat
  Thermostat(
    const char* nameString,
    int heaterPin,
    adc1_channel_t tempChannel,
    int neopixelPin,
    int neopixelPowerPin
  ):
    heater(heaterPin),
    tempSensor(tempChannel),
    status(neopixelPin, neopixelPowerPin)
  {
    // get an initial reading for the current temperature
    double currentTemp = tempSensor.readTemp() / 10.0;

    // initialise the status to default
    status.setColor(WHITE);

    // initialise the characteristics
    name = new Characteristic::Name(nameString);
    currentHeaterState = new Characteristic::CurrentHeatingCoolingState(0);
    currentTemperature = new Characteristic::CurrentTemperature(currentTemp);
    targetHeaterState = new Characteristic::TargetHeatingCoolingState(0, true);
    targetTemperature = new Characteristic::TargetTemperature(20, true);
    temperatureDisplayUnits = new Characteristic::TemperatureDisplayUnits(0);
    coolingThresholdTemperature = new Characteristic::CoolingThresholdTemperature(24, true);
    heatingThresholdTemperature = new Characteristic::HeatingThresholdTemperature(18, true);

    // setup the characteristics
    targetHeaterState->setValidValues(3, 0, 1, 3);
    temperatureDisplayUnits->setValidValues(1, 0);
    temperatureDisplayUnits->removePerms(PW);
    currentTemperature->setRange(-270, 100, 0.1);
    targetTemperature->setRange(10, 38, 0.5);
    coolingThresholdTemperature->setRange(10, 35, 0.5);
    heatingThresholdTemperature->setRange(0, 25, 0.5);

    // setup the state of the thermostat
    lastSenseTemperature = millis();
    lastUpdateState = millis();
    tempReadings = 0;
    tempSum = 0;
  }

  // update the state of the thermostat
  void loop() {
    // update current temperature when you have enough readings
    if (tempReadings > 50) {
      double currentTemp = (tempSum / tempReadings) / 10.0;
      currentTemperature->setVal(currentTemp);
      tempReadings = 0;
      tempSum = 0;
    }

    // update state every 5 seconds
    if ((millis() - lastUpdateState) > 5000) {
      // get the current state of the system
      int targetState = targetHeaterState->getVal();
      bool heaterState = currentHeaterState->getVal();
      double targetTemp = targetTemperature->getVal<double>(),
             currentTemp = currentTemperature->getVal<double>(),
             minTemp = heatingThresholdTemperature->getVal<double>(),
             maxTemp = coolingThresholdTemperature->getVal<double>();

      // decide whether to changed the state of the heater
      // based on currently set config
      bool toggle = false;
      switch (targetState) {
        case 0:
          toggle = heaterState;
          break;
        case 1:
          toggle = toggleHeaterState(currentTemp, targetTemp, targetTemp, heaterState);
          break;
        case 3:
          toggle = toggleHeaterState(currentTemp, minTemp, maxTemp, heaterState);
      }

      // decide the status color of the LED
      // based on currently set config
      color_t color = PURPLE;
      switch (targetState) {
        case 0:
          color = PURPLE;
          break;
        case 1:
          color = statusColor(currentTemp, targetTemp - 1, targetTemp + 1);
          break;
        case 3:
          color = statusColor(currentTemp, minTemp, maxTemp);
      }

      // set the color of the status LED and
      // toggle the state of the heater
      status.setColor(color);
      if (toggle) {
        currentHeaterState->setVal(!heaterState);
        heater.setState(!heaterState);
      }

      // update time
      lastUpdateState = millis();
    }

    // sense temperature every 100 milliseconds
    if ((millis() - lastSenseTemperature) > 100) {
      // read the temperature and validate before accepting
      int reading = tempSensor.readTemp();
      if (reading >= -10 * 10 && reading <= 30 * 10) {
        tempReadings += 1;
        tempSum += reading;
      }

      // update time
      lastSenseTemperature = millis();
    }
  }

  // whether to toggle heater state
  bool toggleHeaterState(double currentTemp, double minTemp, double maxTemp, bool heaterState) {
    return ((currentTemp < minTemp && !heaterState) || (currentTemp > maxTemp && heaterState));
  }

  // return the color of the status LED
  color_t statusColor(double currentTemp, double minTemp, double maxTemp) {
    if (currentTemp < minTemp) {
      return BLUE;
    } else if (currentTemp > maxTemp) {
      return RED;
    } else {
      return GREEN;
    }
  }
};
