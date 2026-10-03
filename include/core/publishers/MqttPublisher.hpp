#pragma once

#include <Arduino.h>

#include "core/publishers/Publisher.hpp"

// =============================================================================
// SEMA — Publicador MQTT
// =============================================================================
// D-0010 / README §56. Publica el Modelo Canónico como JSON en un topic.
// Queda deshabilitado si el host está vacío.

namespace sema {

class MqttPublisher : public Publisher {
public:
  MqttPublisher(const char* id, const char* host, uint16_t port, const char* topic);

  const char* id() const override;
  bool enabled() const override;
  bool publish(const Measurement& m) override;

private:
  const char* id_;
  String host_;
  uint16_t port_;
  String topic_;
};

}  // namespace sema
