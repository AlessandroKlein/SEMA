#include "core/prediction/Forecast.hpp"

#include <math.h>
#include <string.h>

namespace sema {

namespace {

// --- Parámetros del modelo (float, sin heap) ---

// Confianza decae con el horizonte: conf = 1 / (1 + h / kHorizonDecayHours).
constexpr float kHorizonDecayHours = 12.0f;

// Span mínimo de la ventana (horas) para confiar en la pendiente.
constexpr float kSpanFullHours = 6.0f;

// Muestras por bin necesarias para usar el promedio diurno.
constexpr float kDiurnalMinSamples = 3.0f;

// Umbral "está lloviendo" (mm/h). Por debajo se persiste "seco".
constexpr float kWetRateMmH = 0.05f;

// Decaimiento de la persistencia de lluvia (la lluvia no dura 24 h).
constexpr float kRainDecayHours = 3.0f;

// Nº de muestras con el que el factor de datos llega a 1.
constexpr float kFullDataSamples = 12.0f;

float clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }

}  // namespace

Forecast::Mag Forecast::toMag(const char* measurement) {
  if (measurement == nullptr) return Mag::Count;
  if (strcmp(measurement, "temperature") == 0) return Mag::Temperature;
  if (strcmp(measurement, "humidity") == 0) return Mag::Humidity;
  if (strcmp(measurement, "rain") == 0 || strcmp(measurement, "precipitation") == 0) {
    return Mag::Rain;
  }
  return Mag::Count;
}

uint8_t Forecast::hourOfDay(uint32_t ts) {
  return static_cast<uint8_t>((ts % 86400u) / 3600u);
}

void Forecast::push(Series& s, uint32_t ts, float value) {
  s.ring[s.head].ts = ts;
  s.ring[s.head].value = value;
  s.head = static_cast<uint8_t>((s.head + 1) % kTrendMax);
  if (s.count < kTrendMax) ++s.count;
}

bool Forecast::trend(const Series& s, float& slope, float& intercept) const {
  const uint8_t n = s.count;
  if (n < 2) return false;

  // La última muestra es la de referencia (x=0). Las anteriores tienen x <= 0.
  const uint32_t lastTs = s.ring[(s.head + kTrendMax - 1) % kTrendMax].ts;

  float sumX = 0.0f, sumY = 0.0f, sumXX = 0.0f, sumXY = 0.0f;
  for (uint8_t i = 0; i < n; ++i) {
    const Sample& smp = s.ring[(s.head + kTrendMax - n + i) % kTrendMax];
    // Diferencia en horas; int64 evita el desbordamiento de uint32 al restar.
    const float x = static_cast<float>(static_cast<int64_t>(smp.ts) -
                                       static_cast<int64_t>(lastTs)) /
                    3600.0f;
    sumX += x;
    sumY += smp.value;
    sumXX += x * x;
    sumXY += x * smp.value;
  }

  const float denom = static_cast<float>(n) * sumXX - sumX * sumX;
  if (fabsf(denom) < 1e-6f) {
    // Ventana degenerada (todo en el mismo instante): sin pendiente, media.
    slope = 0.0f;
    intercept = sumY / static_cast<float>(n);
    return true;
  }
  slope = (static_cast<float>(n) * sumXY - sumX * sumY) / denom;
  intercept = (sumY - slope * sumX) / static_cast<float>(n);
  return true;
}

void Forecast::addSample(const char* measurement, uint32_t ts, float value) {
  const Mag mag = toMag(measurement);
  if (mag >= Mag::Count) return;

  push(series_[static_cast<uint8_t>(mag)], ts, value);

  // El promedio diurno sólo aplica a temperatura y humedad.
  if (mag == Mag::Temperature || mag == Mag::Humidity) {
    const uint8_t di = (mag == Mag::Temperature) ? 0 : 1;
    DiurnalBin& bin = diurnal_[di][hourOfDay(ts)];
    bin.sum += value;
    if (bin.count < 0xFFFF) ++bin.count;
  }
}

bool Forecast::predict(const char* measurement, uint32_t horizonHours,
                       float& value, float& confidence) {
  const Mag mag = toMag(measurement);
  if (mag >= Mag::Count) return false;

  const uint32_t h = horizonHours < kMinHorizon   ? kMinHorizon
                     : (horizonHours > kMaxHorizon ? kMaxHorizon
                                                   : horizonHours);

  const Series& s = series_[static_cast<uint8_t>(mag)];
  if (s.count < kMinSamples) return false;

  // Índices de la última y la más antigua muestras de la ventana.
  const uint8_t lastIdx = (s.head + kTrendMax - 1) % kTrendMax;
  const uint8_t firstIdx = (s.head + kTrendMax - s.count) % kTrendMax;
  const uint32_t lastTs = s.ring[lastIdx].ts;
  const float lastVal = s.ring[lastIdx].value;
  const float spanHours =
      static_cast<float>(static_cast<int64_t>(lastTs) -
                         static_cast<int64_t>(s.ring[firstIdx].ts)) /
      3600.0f;

  // Factores comunes de confianza.
  const float dataFactor = clamp01(static_cast<float>(s.count) / kFullDataSamples);
  const float spanFactor = clamp01(spanHours / kSpanFullHours);
  const float horizonFactor = 1.0f / (1.0f + static_cast<float>(h) / kHorizonDecayHours);

  if (mag == Mag::Rain) {
    // Tasa reciente (mm/h) sobre la ventana. Los contadores de lluvia son
    // acumulativos; un reset se ve como tasa negativa → se descarta.
    float rate = spanHours > 0.0f ? (lastVal - s.ring[firstIdx].value) / spanHours
                                  : 0.0f;
    if (rate < 0.0f) rate = 0.0f;

    if (rate >= kWetRateMmH) {
      // Lloviendo: extrapolar la tasa con decaimiento exponencial simple.
      const float decay = 1.0f / (1.0f + static_cast<float>(h) / kRainDecayHours);
      value = rate * static_cast<float>(h) * decay;
    } else {
      // Seco: persistencia de "sin lluvia".
      value = 0.0f;
    }
    confidence = dataFactor * spanFactor * horizonFactor;
    return true;
  }

  // --- Temperatura / humedad ---
  float slope = 0.0f, intercept = lastVal;
  const bool hasTrend = trend(s, slope, intercept);
  const float trendVal = hasTrend ? intercept + slope * static_cast<float>(h)
                                  : lastVal;

  const uint8_t di = (mag == Mag::Temperature) ? 0 : 1;
  const uint8_t targetHour = hourOfDay(lastTs + h * 3600u);
  const DiurnalBin& bin = diurnal_[di][targetHour];

  if (bin.count >= kDiurnalMinSamples) {
    // Mezcla tendencia (domina a corto plazo) con patrón diurno (domina a 24 h).
    const float diurnalVal = bin.sum / static_cast<float>(bin.count);
    const float blend = clamp01(static_cast<float>(h) / 24.0f);
    value = trendVal * (1.0f - blend) + diurnalVal * blend;
    // Con patrón diurno la confianza decae menos con el horizonte.
    confidence = dataFactor * spanFactor * (0.4f + 0.6f * horizonFactor);
  } else {
    value = trendVal;
    confidence = dataFactor * spanFactor * horizonFactor;
  }

  return true;
}

void Forecast::reset() {
  // count=0 ya deja todas las lecturas inertes; no hace falta limpiar `ring`.
  for (uint8_t i = 0; i < kMagnitudes; ++i) {
    series_[i].head = 0;
    series_[i].count = 0;
  }
  for (uint8_t m = 0; m < 2; ++m) {
    for (uint8_t h = 0; h < kDiurnalBins; ++h) {
      diurnal_[m][h].sum = 0.0f;
      diurnal_[m][h].count = 0;
    }
  }
}

uint8_t Forecast::sampleCount(const char* measurement) const {
  const Mag mag = toMag(measurement);
  if (mag >= Mag::Count) return 0;
  return series_[static_cast<uint8_t>(mag)].count;
}

// =============================================================================
// SELF-CHECK (manual, sin hardware — correr en un host o con un test unitario):
//
//   Forecast f;
//   // Temperatura subiendo 1 °C/h: muestra i-ésima = 20 + i (última = 43 °C).
//   for (int i = 0; i < 24; ++i)
//     f.addSample("temperature", 1000000u + (uint32_t)i * 3600u, 20.0f + i);
//   float v, c;
//   f.predict("temperature", 3, v, c);
//   // Esperado: v ≈ 46.0 (última 43 + pendiente 1 °C/h · 3 h; sin patrón diurno
//   // aún: cada bin tiene 1 muestra < kDiurnalMinSamples=3) y c ≈ 0.8
//   // (dataFactor=1, spanFactor=1, horizonFactor=1/(1+3/12)=0.8). VERIFICADO.
//
//   f.reset();
//   // Lluvia acumulando 2 mm/h (0,2,4,6,8,10 mm en horas 0..5).
//   for (int i = 0; i < 6; ++i)
//     f.addSample("rain", 1000000u + (uint32_t)i * 3600u, (float)i * 2.0f);
//   f.predict("rain", 1, v, c);
//   // Esperado: v ≈ 1.5 mm (tasa 2 mm/h · 1 h · decaimiento 1/(1+1/3)=0.75)
//   // y c ≈ 0.385 (dataFactor 0.5 · spanFactor 0.833 · horizonFactor 0.923).
//   // VERIFICADO.
// =============================================================================

}  // namespace sema
