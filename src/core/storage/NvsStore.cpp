#include "core/storage/NvsStore.hpp"

#include <Preferences.h>

namespace sema {

NvsStore::NvsStore() : prefs_(new Preferences()) {}

NvsStore::~NvsStore() {
  delete prefs_;
}

bool NvsStore::begin(const char* name) {
  return prefs_ != nullptr && prefs_->begin(name, false);
}

bool NvsStore::getString(const char* key, String& out) {
  if (prefs_ == nullptr || !prefs_->isKey(key)) {
    return false;
  }
  out = prefs_->getString(key, "");
  return true;
}

bool NvsStore::putString(const char* key, const char* value) {
  return prefs_ != nullptr && prefs_->putString(key, value) > 0;
}

bool NvsStore::getUInt(const char* key, uint32_t& out) {
  if (prefs_ == nullptr || !prefs_->isKey(key)) {
    return false;
  }
  out = prefs_->getUInt(key, 0);
  return true;
}

bool NvsStore::putUInt(const char* key, uint32_t value) {
  return prefs_ != nullptr && prefs_->putUInt(key, value) > 0;
}

bool NvsStore::clear() {
  return prefs_ != nullptr && prefs_->clear();
}

}  // namespace sema
