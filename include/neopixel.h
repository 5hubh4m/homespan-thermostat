#include <Adafruit_NeoPixel.h>

// declare the color type
typedef uint32_t color_t;

// define colors to use with the LED
#define RED Adafruit_NeoPixel::Color(255, 0, 0)
#define BLUE Adafruit_NeoPixel::Color(0, 0, 255)
#define GREEN Adafruit_NeoPixel::Color(0, 255, 0)
#define WHITE Adafruit_NeoPixel::Color(255, 255, 255)
#define PURPLE Adafruit_NeoPixel::Color(128, 0, 128)

// this class encapsulates the NeoPixel LED
class NeoPixel {
private:
  // the NeoPixel object
  Adafruit_NeoPixel pixel;

public:
  // construct the NeoPixel object
  NeoPixel(): pixel(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800) {
    // turn on power to the NeoPixel LED
    pinMode(NEOPIXEL_POWER, OUTPUT);
    digitalWrite(NEOPIXEL_POWER, HIGH);

    // start the NeoPixel library
    pixel.begin();
  }

  // set the color of the NeoPixel LED
  void setColor(color_t c) {
    pixel.fill(c);
    pixel.show();
  }
};
