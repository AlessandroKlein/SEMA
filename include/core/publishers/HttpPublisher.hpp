#pragma once

#include <Arduino.h>

#include "core/publishers/Publisher.hpp"

// =============================================================================
// SEMA — Publicador HTTP genérico (webhook)
// =============================================================================
// D-0047 / README §113. Envía el Modelo Canónico como JSON por HTTP POST a una
// URL configurable. Si la URL está vacía queda deshabilitado.

namespace sema {

class HttpPublisher : public Publisher {
public:
  HttpPublisher(const char* id, const char* url);

  void setUrl(const char* url) { url_ = url; }

  const char* id() const override;
  bool enabled() const override;
  bool publish(const Measurement& m) override;

private:
  const char* id_;
  String url_;
};

}  // namespace sema
