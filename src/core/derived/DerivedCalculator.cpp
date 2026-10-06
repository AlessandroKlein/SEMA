#include "core/derived/DerivedCalculator.hpp"

#include <Arduino.h>
#include <math.h>

#include "core/Time.hpp"

namespace sema {

namespace {

float dewPoint(float tC, float rh) {
  const float a = 17.27f;
  const float b = 237.7f;
  const float gamma = a * tC / (b + tC) + logf(rh / 100.0f);
  return b * gamma / (a - gamma);
}

float heatIndexC(float tC, float rh) {
  const float tF = tC * 9.0f / 5.0f + 32.0f;
  const float hi = -42.379f + 2.04901523f * tF + 10.14333127f * rh -
                   0.22475541f * tF * rh - 0.00683783f * tF * tF -
                   0.05481717f * rh * rh + 0.00122874f * tF * tF * rh +
                   0.00085282f * tF * rh * rh - 0.00000199f * tF * tF * rh * rh;
  return (hi - 32.0f) * 5.0f / 9.0f;
}

float windChillC(float tC, float windMs) {
  const float tF = tC * 9.0f / 5.0f + 32.0f;
  const float vMph = windMs * 2.23694f;
  if (tF > 50.0f || vMph < 3.0f) {
    return tC;  // fuera de rango: no aplica
  }
  const float wc = 35.74f + 0.6215f * tF - 35.75f * powf(vMph, 0.16f) +
                   0.4275f * tF * powf(vMph, 0.16f);
  return (wc - 32.0f) * 5.0f / 9.0f;
}

float qnh(float pressureHpa, float altitudeM, float tC) {
  return pressureHpa * powf(1.0f + 0.0065f * altitudeM / (tC + 273.15f), 5.257f);
}

float vpd(float tC, float rh) {
  const float es = 0.6108f * expf(17.27f * tC / (tC + 237.3f));  // kPa
  return es * (1.0f - rh / 100.0f);
}

float aqiFromPm25(float pm25) {
  struct BP { float lo, hi, aqiLo, aqiHi; };
  static const BP bps[] = {
      {0.0f, 12.0f, 0.0f, 50.0f},       {12.1f, 35.4f, 51.0f, 100.0f},
      {35.5f, 55.4f, 101.0f, 150.0f},   {55.5f, 150.4f, 151.0f, 200.0f},
      {150.5f, 250.4f, 201.0f, 300.0f}, {250.5f, 350.4f, 301.0f, 400.0f},
      {350.5f, 500.4f, 401.0f, 500.0f},
  };
  for (const BP& bp : bps) {
    if (pm25 <= bp.hi) {
      return bp.aqiLo + (bp.aqiHi - bp.aqiLo) * (pm25 - bp.lo) / (bp.hi - bp.lo);
    }
  }
  return 500.0f;
}

float barometricAltitude(float pressureHpa) {
  return 44330.0f * (1.0f - powf(pressureHpa / 1013.25f, 1.0f / 5.255f));
}

void emit(std::vector<Measurement>& out, const char* measurement, float value,
          const char* unit) {
  Measurement m;
  m.sensorId = "DERIVED";
  m.channelId = measurement;
  m.measurement = measurement;
  m.value = value;
  m.unit = unit;
  m.quality = Quality::Valid;
  m.timestamp = nowEpoch();
  out.push_back(m);
}

bool hasRaw(const std::vector<Measurement>& raw, const char* measurement) {
  for (const Measurement& m : raw) {
    if (m.measurement == measurement && m.quality == Quality::Valid) {
      return true;
    }
  }
  return false;
}

// 16 posiciones de la veleta WH-SP-WD: 8 resistencias directas + 8 en paralelo.
void windVanePositions(const SystemConfig& sys, float angles[16],
                       uint16_t adcVals[16]) {
  for (int i = 0; i < 8; ++i) {
    const float rd = sys.windResistors[i];
    angles[2 * i] = i * 45.0f;
    adcVals[2 * i] =
        (uint16_t)lroundf(4095.0f * rd / (rd + sys.windRpull));

    const float rn = sys.windResistors[(i + 1) % 8];
    const float rp = (rd * rn) / (rd + rn);
    angles[2 * i + 1] = i * 45.0f + 22.5f;
    adcVals[2 * i + 1] =
        (uint16_t)lroundf(4095.0f * rp / (rp + sys.windRpull));
  }
}

}  // namespace

float DerivedCalculator::windVaneRawAngle(uint16_t adc, const SystemConfig& sys) {
  float angles[16];
  uint16_t adcVals[16];
  windVanePositions(sys, angles, adcVals);

  int best = 0;
  uint32_t bestDiff = 0xFFFFFFFFu;
  for (int i = 0; i < 16; ++i) {
    const uint32_t d = (uint32_t)abs((int)adc - (int)adcVals[i]);
    if (d < bestDiff) {
      bestDiff = d;
      best = i;
    }
  }
  return angles[best];
}

float DerivedCalculator::windDirection(uint16_t adc, const SystemConfig& sys) {
  float a = windVaneRawAngle(adc, sys) - sys.windNorthOffset;
  while (a < 0.0f) a += 360.0f;
  while (a >= 360.0f) a -= 360.0f;
  return a;
}

float DerivedCalculator::convertUnit(float value, const String& measurement,
                                     const String& unit, bool imperial,
                                     String& outUnit) {
  outUnit = unit;
  if (!imperial) {
    return value;
  }
  if (measurement == "temperature" || measurement == "dew_point" ||
      measurement == "heat_index" || measurement == "wind_chill") {
    outUnit = "degF";
    return value * 9.0f / 5.0f + 32.0f;
  }
  if (measurement == "pressure" || measurement == "qnh") {
    outUnit = "inHg";
    return value / 33.8639f;
  }
  if (measurement == "wind_speed" || measurement == "wind_gust") {
    outUnit = "mph";
    return value * 2.23694f;
  }
  if (measurement == "rain" || measurement == "precipitation") {
    outUnit = "in";
    return value / 25.4f;
  }
  if (measurement == "rain_rate") {
    outUnit = "in/h";
    return value / 25.4f;
  }
  if (measurement == "barometric_altitude") {
    outUnit = "ft";
    return value * 3.28084f;
  }
  return value;
}

void DerivedCalculator::compute(const std::vector<Measurement>& raw,
                                std::vector<Measurement>& out,
                                const String& units) {
  const bool imperial = (units == "imperial");

  // --- Extrae las magnitudes crudas disponibles (sensor encendido). ---
  float t = NAN, rh = NAN, p = NAN, wind = NAN, rain = NAN, pm25 = NAN, pm10 = NAN;
  for (const Measurement& m : raw) {
    if (m.quality != Quality::Valid) continue;
    if (m.measurement == "temperature" && isnan(t)) t = m.value;
    else if (m.measurement == "humidity" && isnan(rh)) rh = m.value;
    else if (m.measurement == "pressure" && isnan(p)) p = m.value;
    else if (m.measurement == "wind_speed" && isnan(wind)) wind = m.value;
    else if (m.measurement == "rain" && isnan(rain)) rain = m.value;
    else if (m.measurement == "pm25" && isnan(pm25)) pm25 = m.value;
    else if (m.measurement == "pm10" && isnan(pm10)) pm10 = m.value;
  }

  // --- Magnitudes derivadas (sólo con las entradas disponibles). ---
  String u;
  if (!isnan(t) && !isnan(rh)) {
    emit(out, "dew_point", convertUnit(dewPoint(t, rh), "dew_point", "degC", imperial, u), u.c_str());
    emit(out, "heat_index", convertUnit(heatIndexC(t, rh), "heat_index", "degC", imperial, u), u.c_str());
    emit(out, "vpd", vpd(t, rh), "kPa");
  }
  if (!isnan(t) && !isnan(wind)) {
    emit(out, "wind_chill", convertUnit(windChillC(t, wind), "wind_chill", "degC", imperial, u), u.c_str());
  }
  if (!isnan(p) && !isnan(t)) {
    emit(out, "qnh", convertUnit(qnh(p, sys_.altitude, t), "qnh", "hPa", imperial, u), u.c_str());
    emit(out, "barometric_altitude", convertUnit(barometricAltitude(p), "barometric_altitude", "m", imperial, u), u.c_str());
  }
  if (!isnan(pm25)) {
    emit(out, "aqi", aqiFromPm25(pm25), "index");
  }

  // --- Tasa de lluvia y acumulado (PCNT con channel "rain", mm). ---
  if (!isnan(rain) && hasRaw(raw, "rain")) {
    rainTotal_ += rain;
    const uint32_t ts = nowEpoch();
    float rate = 0.0f;
    if (lastRainTs_ != 0 && ts > lastRainTs_) {
      rate = rain / ((ts - lastRainTs_) / 3600.0f);  // mm/h
    }
    lastRainTs_ = ts;
    emit(out, "rain_rate", convertUnit(rate, "rain_rate", "mm/h", imperial, u), u.c_str());
    emit(out, "rain_accumulated", convertUnit(rainTotal_, "rain", "mm", imperial, u), u.c_str());
  }

  // --- Dirección de viento (veleta WH-SP-WD por tabla de resistencias). ---
  if (sys_.windDirectionPin != 0) {
    const uint16_t adc = analogRead(sys_.windDirectionPin);
    emit(out, "wind_direction", windDirection(adc, sys_), "deg");
  }
}

}  // namespace sema
