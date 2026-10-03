#include "core/derived/DerivedEngine.hpp"

#include <cmath>

namespace sema {

float DerivedEngine::dewPoint(float tempC, float relHum) {
  // Fórmula de Magnus (aproximación habitual).
  const float a = 17.62f;
  const float b = 243.12f;
  const float gamma = logf(relHum / 100.0f) + (a * tempC) / (b + tempC);
  return (b * gamma) / (a - gamma);
}

float DerivedEngine::heatIndex(float tempC, float relHum) {
  // Regresión de Rothfusz (NOAA); válida para T ≳ 27 °C y RH ≳ 40 %.
  const float t = tempC * 9.0f / 5.0f + 32.0f;
  const float hi = -42.379f + 2.04901523f * t + 10.14333127f * relHum
                   - 0.22475541f * t * relHum - 0.00683783f * t * t
                   - 0.05481717f * relHum * relHum + 0.00122874f * t * t * relHum
                   + 0.00085282f * t * relHum * relHum
                   - 0.00000199f * t * t * relHum * relHum;
  return (hi - 32.0f) * 5.0f / 9.0f;
}

void DerivedEngine::compute(std::vector<Measurement>& measurements) {
  // Busca un sensor que aporte temperatura y humedad simultáneamente (mismo id).
  const Measurement* temp = nullptr;
  const Measurement* hum = nullptr;
  for (const Measurement& m : measurements) {
    if (m.channelId != "temperature") {
      continue;
    }
    for (const Measurement& h : measurements) {
      if (h.sensorId == m.sensorId && h.channelId == "humidity") {
        temp = &m;
        hum = &h;
        break;
      }
    }
    if (temp != nullptr) {
      break;
    }
  }
  if (temp == nullptr || hum == nullptr) {
    return;
  }

  const float td = dewPoint(temp->value, hum->value);
  const float hi = heatIndex(temp->value, hum->value);

  Measurement dew;
  dew.sensorId = "DERIVED";
  dew.channelId = "dew_point";
  dew.measurement = "dew_point";
  dew.value = td;
  dew.unit = "degC";
  dew.quality = Quality::Valid;
  dew.sequence = temp->sequence;
  dew.timestamp = temp->timestamp;
  measurements.push_back(dew);

  Measurement heat;
  heat.sensorId = "DERIVED";
  heat.channelId = "heat_index";
  heat.measurement = "heat_index";
  heat.value = hi;
  heat.unit = "degC";
  heat.quality = Quality::Valid;
  heat.sequence = temp->sequence;
  heat.timestamp = temp->timestamp;
  measurements.push_back(heat);
}

}  // namespace sema
