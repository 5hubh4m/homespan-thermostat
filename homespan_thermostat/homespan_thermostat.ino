#include <HomeSpan.h>

// define the constants for the thermostat
#define NAME         "Thermostat"
#define MANUFACTURER "Shubham Chaudhary"
#define MODEL        "Thermostat"
#define SERIAL_NUM   "0x0000001"
#define FIRMWARE     "v1.0.0"

// the pin on which the heater relay and
// temperature sensor is connected
#define HEATER_PIN 10
#define TEMP_PIN   8

// constraint the TEMP_PIN to be between 3 and 10
// since we can only use ADC1 as ADC2 is occupied
// during Wi-Fi usage
#if TEMP_PIN < 3 || TEMP_PIN > 10
#error "Temperature sensor MUST be on ADC1 as ADC2 is occupied during Wi-Fi."
#endif

// threshold determines how sensitive
// the heater is to the current temperature
#define TEMP_THRESHOLD 0.1

// alpha for exponential averaging of
// temperature readings, more is smoother
#define TEMP_EXP_ALPHA 0.9

// period to scan for state changes in milliseconds
#define STATUS_PERIOD 5000

// period to scan for temperature in milliseconds
#define SENSE_PERIOD 50

// uncomment these lines to define your own custom parameters
// #define WIFI_SSID             "Your Wi-Fi SSID"
// #define WIFI_PASSWORD         "Your Wi-Fi Password"
// #define HOMEKIT_PAIRING_CODE  "8 Digit Pairing Code"
// #define HOMEKIT_PAIRING_QR_ID "4 Digit Alphanumeric Code"
// #define DEFAULT_OTA_PASSWORD  "Custom OTA Password"

// include the custom classes
#include "identify.h"
#include "thermostat.h"

void setup() {
  // setup serial
  Serial.begin(115200);

  // setup span device
  homeSpan.enableOTA();
  homeSpan.setStatusPin(LED_BUILTIN);
  homeSpan.setHostNameSuffix("");
  homeSpan.setSketchVersion("1.0.0");

#if defined(WIFI_SSID) && defined(WIFI_PASSWORD)
  // if Wi-Fi credentials are defined, supply
  // them here, otherwise set them using the HomeSpan CLI
  homeSpan.setWifiCredentials(WIFI_SSID, WIFI_PASSWORD);
#endif

#ifdef HOMEKIT_PAIRING_CODE
  // if HomeKit pairing code is defined, supply
  // it here, otherwise the default is used, or
  // can be set using HomeSpan CLI
  homeSpan.setPairingCode(HOMEKIT_PAIRING_CODE);
#endif

#ifdef HOMEKIT_PAIRING_QR_ID
  // if HomeKit QR Code ID is defined, supply
  // it here, otherwise the default is used, or
  // can be set using HomeSpan CLI
  homeSpan.setQRID(HOMEKIT_PAIRING_QR_ID);
#endif

  // start span device
  homeSpan.begin(Category::Thermostats, NAME, NAME, MODEL);

  // initialise span acessory
  new SpanAccessory();

  // initialise accessory information service
  new Identify(NAME, MANUFACTURER, SERIAL_NUM, MODEL, FIRMWARE);

  // intialise the protocol identification service
  new ProtocolVersion();

  // initialise thermostat service with the
  // right name, temperature sensor and heater
  // relay pin and in-built NeoPixel as a status LED
  (new Thermostat(
    TEMP_PIN,
    HEATER_PIN,
    TEMP_EXP_ALPHA,
    TEMP_THRESHOLD,
    STATUS_PERIOD,
    SENSE_PERIOD
  ))->setPrimary();
}

void loop() {
  // run the span IO loop
  homeSpan.poll();
}
