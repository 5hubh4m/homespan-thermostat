// this class encalsulates the relay operation
class Relay {
private:
  // store the pin to which
  // the relay is connected
  int relayPin;

public:
  // constructor for the class sets
  // up the relay in OFF mode
  Relay(int pin): relayPin(pin) {
    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, LOW);
  }

  // set the state of the relay
  void setState(bool state) {
    digitalWrite(relayPin, state);
  }
};
