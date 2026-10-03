#include "core/SemaCore.hpp"

#include <Arduino.h>

namespace sema {

SemaCore& SemaCore::instance() {
  static SemaCore core;
  return core;
}

void SemaCore::setup() {
  Serial.begin(115200);
  delay(200);

  Serial.println();
  Serial.printf("SEMA v%s (hw %s, schema %d, protocol %d)\n",
                SEMA_FW_VERSION,
                SEMA_HW_VERSION,
                SEMA_CONFIG_SCHEMA_VERSION,
                SEMA_PROTOCOL_VERSION);
  Serial.printf("Módulos registrados: %u\n", static_cast<unsigned>(modules_.count()));

  modules_.enableAll();
}

void SemaCore::loop() {
  modules_.loopAll();
}

}  // namespace sema
