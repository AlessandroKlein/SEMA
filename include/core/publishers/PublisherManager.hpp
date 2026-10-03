#pragma once

#include <vector>

#include "core/Measurement.hpp"
#include "core/publishers/Publisher.hpp"

// =============================================================================
// SEMA — Registro de publicadores
// =============================================================================
// D-0010. Conduce los publicadores habilitados después del almacenamiento.

namespace sema {

class PublisherManager {
public:
  void registerPublisher(Publisher* publisher);
  void publishAll(const Measurement& m);

  size_t count() const { return publishers_.size(); }

private:
  std::vector<Publisher*> publishers_;
};

}  // namespace sema
