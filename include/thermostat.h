#include <HomeSpan.h>

#include "relay.h"
#include "neopixel.h"
#include "temperature.h"

// enums to define some constants
enum TargetHeaterStates {
  OFF = 0,
  HEAT = 1,
  COOL = 2, // we disable this state since we don't have a AC
  AUTO = 3
};

enum CurrentHeaterStates {
  IDLE = 0,
  HEATING = 1,
  COOLING = 2 // we disable this state since we don't have an AC (in essence this is just a bool)
};

enum TemperatureDisplayUnits {
  CELSIUS = 0,
  FARHENITE = 1 // we disable this state since we are not heathens
};

// the class that contains the logic for running the
// thermostat service
class Thermostat: public Service::Thermostat {
private:
  // setup the sensor, heater relay, and status LED
  TemperatureSensor sensor;
  NeoPixel status;
  Relay heater;

  // temperature threshold, determines the degree
  // to which heater state change is sensitive to
  // current temperature
  const double tempThreshold;

  // this is the minimum number of temperature
  // readings to accumulate before calculating
  const unsigned int minReadings;

  // period in milliseconds before the state is
  // updated and temperature is sensed respectively
  const unsigned long statusUpdatePeriod;
  const unsigned long temperatureSensePeriod;

  // setup the characteristic objects
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
    int tempPin,
    int heaterPin,
    double threshold,
    unsigned int accumulate,
    unsigned long statePeriod,
    unsigned long sensePeriod
  ):
    sensor(tempPin),
    heater(heaterPin),
    minReadings(accumulate),
    tempThreshold(threshold),
    statusUpdatePeriod(statePeriod),
    temperatureSensePeriod(sensePeriod)
  {
    // get an initial reading for the current temperature
    double currentTemp = sensor.readTemp() / 10.0;

    // initialise the status to default
    status.setColor(WHITE);

    // initialise the characteristics
    currentHeaterState = new Characteristic::CurrentHeatingCoolingState(IDLE);
    currentTemperature = new Characteristic::CurrentTemperature(currentTemp);
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
    lastSenseTemperature = millis();
    lastUpdateState = millis();
    tempReadings = 0;
    tempSum = 0;
  }

  // update the state of the thermostat
  void loop() {
    // sense temperature every given duration
    if ((millis() - lastSenseTemperature) > temperatureSensePeriod) {
      updateTempReadings();
      lastSenseTemperature = millis();
    }

    // update current temperature when you have enough readings
    updateCurrentTemp();

    // update state every given duration
    if ((millis() - lastUpdateState) > statusUpdatePeriod) {
      updateState();
      lastUpdateState = millis();
    }
  }

private:
  // update the state of the system given parameters
  void updateState() {
    // get the current state of the system
    int targetState = targetHeaterState->getVal();
    bool heaterState = currentHeaterState->getVal();
    double targetTemp = targetTemperature->getVal<double>(),
           currentTemp = currentTemperature->getVal<double>(),
           minTemp = heatingThresholdTemperature->getVal<double>(),
           maxTemp = coolingThresholdTemperature->getVal<double>();

    // decide whether to changed the state of the heater
    // based on currently set config
    bool toggle;
    switch (targetState) {
      case OFF:
        toggle = heaterState;
        break;
      case HEAT:
        toggle = toggleHeaterState(currentTemp, targetTemp - 1, targetTemp + 1, heaterState);
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
      heater.setState(!heaterState);
    }
  }

  // accumulate a new temperature reading
  void updateTempReadings() {
    // read the current temperature
    int reading = sensor.readTemp();

    // only accumulate if the reading is valid
    if (reading >= -10 * 10 && reading <= 30 * 10) {
      tempReadings += 1;
      tempSum += reading;
    }
  }

  // update the current temperature from accumulated readings
  // if enough reaadings have been accumulated so far
  void updateCurrentTemp() {
    if (tempReadings > minReadings) {
      double currentTemp = (tempSum / tempReadings) / 10.0;
      currentTemperature->setVal(currentTemp);
      tempReadings = 0;
      tempSum = 0;
    }
  }

  // whether to toggle heater state
  bool toggleHeaterState(double currentTemp, double minTemp, double maxTemp, bool heaterState) {
    return (
      (currentTemp < (minTemp - tempThreshold) && !heaterState) ||
      (currentTemp > (maxTemp + tempThreshold) && heaterState)
    );
  }

  // return the color of the status LED
  color_t statusColor(double currentTemp, double minTemp, double maxTemp) {
    if (currentTemp < (minTemp - tempThreshold)) {
      return BLUE;
    } else if (currentTemp > (maxTemp + tempThreshold)) {
      return RED;
    } else {
      return GREEN;
    }
  }
};
