#pragma once

#include "core/storage/Storage.hpp"

class Preferences;  // forward-declare (global namespace, arduino Preferences)

namespace sema {

// Backend NVS del Storage API (D-0046). Envuelve la librería Preferences del
// core Arduino-ESP32. Usado para configuración, identidad y contadores.
class NvsStore : public KeyValueStore {
public:
  NvsStore();
  ~NvsStore() override;

  bool begin(const char* name) override;

  bool getString(const char* key, String& out) override;
  bool putString(const char* key, const char* value) override;

  bool getUInt(const char* key, uint32_t& out) override;
  bool putUInt(const char* key, uint32_t value) override;

  bool clear() override;

private:
  Preferences* prefs_ = nullptr;
};

}  // namespace sema
