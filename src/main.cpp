#include "sensesp_app_builder.h"
#include "sensesp_app.h"
#include "sensesp/sensors/analog_input.h"
#include "sensesp/transforms/moving_average.h"
#include "sensesp/transforms/curveinterpolator.h"
#include "sensesp/transforms/voltagedivider.h"
#include "sensesp/signalk/signalk_output.h"

using namespace sensesp;

// Define pin constants
const uint8_t kTankSensorPin = 33;  // GPIO pin for analog input

// The setup function performs one-time application initialization
void setup() {
#ifndef SERIAL_DEBUG_DISABLED
  SetupSerialDebug(115200);
#endif

  // Create the global SensESPApp object
  SensESPAppBuilder builder;
  sensesp_app = builder.set_hostname("water-tank-sensor")
  ->enable_ota("Hunger")
                      ->get_app();

  // Create the analog input sensor for reading tank level
  auto* tank_sensor = new AnalogInput(
      kTankSensorPin,     // GPIO pin
      1000,              // Read interval in ms
      "/tank/sensor"     // Configuration path
  );

    // Create the voltage divider transform to convert ADC reading to resistance
  auto voltage_divider = new VoltageDividerR2(
      220.0,            // R1 resistance value
      5.0,              // Input voltage (Vin)
      "/tank/divider"   // Config path
  );

  // Create calibration curve transform
  auto calibration = new CurveInterpolator(
      nullptr,           // Points will be configured via UI
      "/tank/calibration"
  );

  // Add moving average to smooth readings
  auto moving_average = new MovingAverage(10);

  // Connect everything together
  tank_sensor->connect_to(voltage_divider)     // Convert ADC to resistance
             ->connect_to(calibration)          // Convert resistance to level
             ->connect_to(moving_average)       // Smooth the output
             ->connect_to(new SKOutputFloat(    // Send to Signal K
                 "tanks.0.currentLevel",        // Signal K path
                 "/tank/signalk",              // Config path for SK output
                 new SKMetadata("Water Tank Level", "%")  // Value metadata
             ));

}

void loop() {
  event_loop()->tick();
}
