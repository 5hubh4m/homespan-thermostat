#include <HomeSpan.h>

#include "relay.h"
#include "sensor.h"
#include "neopixel.h"

// enums to define some constants
enum TargetHeaterStates {
  OFF = 0,
  HEAT = 1,
  COOL = 2, // we disable this state since we don't have an AC
  AUTO = 3
};

enum CurrentHeaterStates {
  IDLE = 0,
  HEATING = 1,
  COOLING = 2 // we disable this state since we don't have an AC (in essence this is just a bool)
};

enum TemperatureDisplayUnits {
  CELSIUS = 0,
  FAHRENHEIT = 1 // we disable this state since we are not heathens
};

// class ties the heater relay operation to a characteristic
class CurrentHeaterStatus: public Characteristic::CurrentHeatingCoolingState {
private:
  // heater relay
  Relay heater;

public:
  // constructor for the class
  CurrentHeaterStatus(int heaterPin): Characteristic::CurrentHeatingCoolingState(IDLE), heater(heaterPin) {}

  template <typename T>
  void setVal(T value, bool notify = true) {
    heater.setState(value);
    Characteristic::CurrentHeatingCoolingState::setVal(value, notify);
  }
};

// the class that contains the logic for running the
// thermostat service
class Thermostat: public Service::Thermostat {
private:
  // setup the sensor, heater relay, and status LED
  TemperatureSensor sensor;
  NeoPixel status;

  // temperature threshold, determines the degree
  // to which heater state change is sensitive to
  // current temperature
  const float tempThreshold;

  // alpha for exponential averaging temperature
  const float tempExpAlpha;

  // period in milliseconds before the state is
  // updated and temperature is sensed respectively
  const unsigned long statusUpdatePeriod;
  const unsigned long temperatureSensePeriod;

  // setup the characteristic objects
  CurrentHeaterStatus *currentHeaterState;
  SpanCharacteristic *targetHeaterState;
  SpanCharacteristic *targetTemperature;
  SpanCharacteristic *currentTemperature;
  SpanCharacteristic *temperatureDisplayUnits;
  SpanCharacteristic *heatingThresholdTemperature;
  SpanCharacteristic *coolingThresholdTemperature;

  // track the state of the thermostat
  unsigned long lastUpdateTemperature;
  unsigned long lastSenseTemperature;
  unsigned long lastUpdateState;
  float currentExpAvgTemp;
  bool wasUpdated;

public:
  // constructor for the thermostat
  Thermostat(
    int temp,
    int heater,
    float alpha,
    float threshold,
    unsigned long statePeriod,
    unsigned long sensePeriod
  ):
    sensor(temp),
    tempExpAlpha(alpha),
    tempThreshold(threshold),
    statusUpdatePeriod(statePeriod),
    temperatureSensePeriod(sensePeriod)
  {
    // get an initial reading for current temperature
    currentExpAvgTemp = sensor.readTemp();

    // initialise the status to default
    status.setColor(WHITE);

    // initialise the characteristics
    currentHeaterState = new CurrentHeaterStatus(heater);
    currentTemperature = new Characteristic::CurrentTemperature(currentExpAvgTemp);
    targetHeaterState = new Characteristic::TargetHeatingCoolingState(OFF, true);
    targetTemperature = new Characteristic::TargetTemperature(20, true);
    temperatureDisplayUnits = new Characteristic::TemperatureDisplayUnits(CELSIUS);
    coolingThresholdTemperature = new Characteristic::CoolingThresholdTemperature(24, true);
    heatingThresholdTemperature = new Characteristic::HeatingThresholdTemperature(18, true);

    // setup the valid values for characteristics
    currentHeaterState->setValidValues(2, IDLE, HEATING);
    targetHeaterState->setValidValues(3, OFF, HEAT, AUTO);
    temperatureDisplayUnits->setValidValues(1, CELSIUS);
    temperatureDisplayUnits->removePerms(PW);

    // set the ranges and step values of temperatures
    targetTemperature->setRange(10, 38, 1);
    currentTemperature->setRange(-270, 100, 0.1);
    coolingThresholdTemperature->setRange(10, 35, 1);
    heatingThresholdTemperature->setRange(0, 25, 1);

    // setup the state of the thermostat
    lastUpdateTemperature = millis();
    lastSenseTemperature = millis();
    lastUpdateState = millis();
    wasUpdated = false;
  }

  // mark the state to be updated
  // so the next time loop runs it 
  // doesn't wait for timer period
  virtual bool update() override {
    wasUpdated = targetHeaterState->updated() ||
                 targetTemperature->updated() ||
                 coolingThresholdTemperature->updated() ||
                 heatingThresholdTemperature->updated();
    return true;
  }

  // update the state of the thermostat
  virtual void loop() override {
    // sense temperature every given duration
    if ((millis() - lastSenseTemperature) > temperatureSensePeriod) {
      updateTempReading();
      lastSenseTemperature = millis();
    }

    // update current temperature every given duration
    // but only when when you have enough readings
    if ((millis() - lastUpdateTemperature) > statusUpdatePeriod) {
      updateCurrentTemp();
      lastUpdateTemperature = millis();
    }

    // update state every given duration
    if ((millis() - lastUpdateState) > statusUpdatePeriod || wasUpdated) {
      updateState();
      wasUpdated = false;
      lastUpdateState = millis();
    }
  }

private:
  // update the state of the system given parameters
  void updateState() {
    // get the current state of the system
    int targetState = targetHeaterState->getVal();
    bool heaterState = currentHeaterState->getVal();
    float currentTemp = currentExpAvgTemp,
          targetTemp = targetTemperature->getVal<float>(),
          minTemp = heatingThresholdTemperature->getVal<float>(),
          maxTemp = coolingThresholdTemperature->getVal<float>();

    // decide whether to changed the state of the heater
    // based on currently set config
    bool toggle;
    switch (targetState) {
      case OFF:
        toggle = heaterState;
        break;
      case HEAT:
        toggle = toggleHeaterState(currentTemp, targetTemp, targetTemp, heaterState);
        break;
      case AUTO:
      default:
        toggle = toggleHeaterState(currentTemp, minTemp, maxTemp, heaterState);
    }

    // decide the status color of the LED
    // based on currently set config
    color_t color;
    switch (targetState) {
      case OFF:
        color = PURPLE;
        break;
      case HEAT:
        color = statusColor(currentTemp, targetTemp - 1, targetTemp + 1);
        break;
      case AUTO:
      default:
        color = statusColor(currentTemp, minTemp, maxTemp);
    }

    // set the color of the status LED and
    // toggle the state of the heater
    status.setColor(color);
    if (toggle) {
      currentHeaterState->setVal(!heaterState);
    }
  }

  // accumulate a new temperature reading
  // using exponential averaging
  void updateTempReading() {
    // read the current temperature
    float reading = sensor.readTemp();

    // validate for correct seaming reading
    // and accumulate if so
    if (-10 <= reading && reading <= 40) {
      currentExpAvgTemp *= tempExpAlpha;
      currentExpAvgTemp += (1 - tempExpAlpha) * reading;
    }
  }

  // update the current temperature from accumulated readings
  void updateCurrentTemp() {
    currentTemperature->setVal(currentExpAvgTemp);
  }

  // whether to toggle heater state
  bool toggleHeaterState(float currentTemp, float minTemp, float maxTemp, bool heaterState) {
    return (
      (currentTemp < (minTemp + tempThreshold) && !heaterState) ||
      (currentTemp > (maxTemp - tempThreshold) && heaterState)
    );
  }

  // return the color of the status LED
  color_t statusColor(float currentTemp, float minTemp, float maxTemp) {
    if (currentTemp < (minTemp + tempThreshold)) {
      return BLUE;
    } else if (currentTemp > (maxTemp - tempThreshold)) {
      return RED;
    } else {
      return GREEN;
    }
  }
};
