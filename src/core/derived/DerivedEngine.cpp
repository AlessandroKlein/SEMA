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

float DerivedEngine::saturationVaporPressure(float tempC) {
  // Magnus: presión de vapor de saturación en hPa.
  return 6.112f * expf((17.67f * tempC) / (tempC + 243.5f));
}

float DerivedEngine::absoluteHumidity(float tempC, float relHum) {
  // Humedad absoluta en g/m³ a partir de la presión de vapor real.
  const float e = saturationVaporPressure(tempC) * relHum / 100.0f;
  return 216.7f * e / (tempC + 273.15f);
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

  Measurement vpres;
  vpres.sensorId = "DERIVED";
  vpres.channelId = "vapor_pressure";
  vpres.measurement = "vapor_pressure";
  vpres.value = saturationVaporPressure(temp->value) * hum->value / 100.0f;
  vpres.unit = "hPa";
  vpres.quality = Quality::Valid;
  vpres.sequence = temp->sequence;
  vpres.timestamp = temp->timestamp;
  measurements.push_back(vpres);

  Measurement ahum;
  ahum.sensorId = "DERIVED";
  ahum.channelId = "absolute_humidity";
  ahum.measurement = "absolute_humidity";
  ahum.value = absoluteHumidity(temp->value, hum->value);
  ahum.unit = "g/m3";
  ahum.quality = Quality::Valid;
  ahum.sequence = temp->sequence;
  ahum.timestamp = temp->timestamp;
  measurements.push_back(ahum);
}

}  // namespace sema
