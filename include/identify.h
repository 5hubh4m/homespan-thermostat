#include <HomeSpan.h>

// this class defines the identification service
struct Identify: Service::AccessoryInformation {
  // constructor for the service
  Identify(
    const char* name,
    const char* manufacturer,
    const char* serialNumber,
    const char* model,
    const char* firmwareVersion
  ): Service::AccessoryInformation() {
    new Characteristic::Name(name);
    new Characteristic::Manufacturer(manufacturer);
    new Characteristic::SerialNumber(serialNumber);
    new Characteristic::Model(model);
    new Characteristic::FirmwareRevision(firmwareVersion);
    new Characteristic::Identify();
  }
};

// this class defines the protocol information
struct ProtocolVersion: Service::HAPProtocolInformation {
  // constructor for the service
  ProtocolVersion(const char* version = "1.1.0"): Service::HAPProtocolInformation() {
    new Characteristic::Version(version);
  }
};
