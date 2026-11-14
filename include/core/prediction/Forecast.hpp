#pragma once

#include <cstdint>

// =============================================================================
// SEMA — Pronóstico a corto plazo (Forecast)
// =============================================================================
// Predicción ligera de tendencia/pronóstico para "temperature", "humidity" y
// "rain" a horizontes cortos (1..24 h), pensada para mostrarse en la web.
// Sin librerías pesadas ni heap: sólo aritmética float y buffers de tamaño
// fijo, acotados en tiempo de compilación.
//
// Algoritmo (defendible y explicable):
//   - Regresión lineal sobre una ventana deslizante (últimas N muestras) para
//     capturar la tendencia.
//   - Persistencia (último valor) como línea base y fallback.
//   - Promedio diurno (media por hora del día, 24 bins) para temperatura y
//     humedad; se mezcla con la tendencia cuanto más largo es el horizonte.
//   - Precipitación: extrapola la tasa reciente (mm/h) con decaimiento (la
//     lluvia no dura 24 h); persistencia seco/húmedo.
//   - Confianza (0..1) decae con el horizonte y con pocos datos.
//
// Coste de memoria (constante, sin heap):
//   - Series: 3 magnitudes × kTrendMax(64) muestras × 8 B  = 1536 B
//   - Diurno: 2 magnitudes × 24 bins × 8 B                  =  384 B
//   - Estado (índices)                                       =  ~16 B
//   Total ≈ 2 KB. Ajustable reduciendo kTrendMax.
//
// Integración prevista (fuera de este módulo): el endpoint /api/v1/forecast y
// el wiring en SemaCore lo hará otro agente; aquí sólo se entrega el módulo
// autocontenido.
// =============================================================================

namespace sema {

class Forecast {
public:
  // Añade una muestra (timestamp epoch seconds UTC). Magnitudes reconocidas:
  // "temperature", "humidity", "rain" (alias "precipitation"). El resto se
  // ignora silenciosamente.
  void addSample(const char* measurement, uint32_t ts, float value);

  // Predice el valor dentro de `horizonHours` (clamp a 1..24). Devuelve false
  // si la magnitud es desconocida o no hay datos suficientes.
  //   - temperature/humidity: `value` en la misma unidad que las muestras.
  //   - rain: `value` = precipitación adicional esperada (mm) en el horizonte.
  // `confidence` queda en [0..1] sólo cuando devuelve true.
  bool predict(const char* measurement, uint32_t horizonHours,
               float& value, float& confidence);

  // Limpia todo el estado (series + acumuladores diurnos).
  void reset();

  // Número de muestras actualmente buffereadas para una magnitud (diagnóstico).
  uint8_t sampleCount(const char* measurement) const;

private:
  static constexpr uint8_t kMagnitudes = 3;  // temperature, humidity, rain
  static constexpr uint8_t kTrendMax = 64;   // muestras por magnitud
  static constexpr uint8_t kDiurnalBins = 24;  // una por hora del día (UTC)
  static constexpr uint8_t kMinSamples = 6;  // mínimo para poder predecir
  static constexpr uint32_t kMinHorizon = 1;  // horas
  static constexpr uint32_t kMaxHorizon = 24;

  enum class Mag : uint8_t { Temperature = 0, Humidity, Rain, Count };

  struct Sample {
    uint32_t ts;     // epoch seconds UTC
    float value;
  };

  // Ventana deslizante (ring buffer) por magnitud.
  struct Series {
    Sample ring[kTrendMax];
    uint8_t head = 0;   // índice del próximo hueco libre
    uint8_t count = 0;  // muestras válidas (0..kTrendMax)
  };

  // Acumulador diurno: suma y nº de muestras por hora del día.
  struct DiurnalBin {
    float sum = 0.0f;
    uint16_t count = 0;
  };

  static Mag toMag(const char* measurement);
  static uint8_t hourOfDay(uint32_t ts);  // UTC

  void push(Series& s, uint32_t ts, float value);

  // Regresión lineal por mínimos cuadrados sobre la serie. `slope` en unidades
  // por hora; `intercept` es el valor en x=0 (momento de la última muestra).
  // Devuelve false si hay < 2 muestras.
  bool trend(const Series& s, float& slope, float& intercept) const;

  Series series_[kMagnitudes];
  DiurnalBin diurnal_[2][kDiurnalBins];  // [0]=temperature, [1]=humidity
};

}  // namespace sema
