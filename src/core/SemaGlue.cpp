// =============================================================================
// SEMA — Glue de dominio (Fase 2)
// =============================================================================
// Implementa sema::registerDomainHooks() rellenando base::coreHooks(). Todo el
// conocimiento específico de SEMA (catálogo de sensores, magnitudes derivadas,
// pronóstico, calibraciones/reglas por defecto, endpoints web de dominio) vive
// aquí; la base ESP32-Base queda genérica y desacoplada.

#include "core/SemaGlue.hpp"

#include <Arduino.h>
#include <Wire.h>
#include <math.h>

#include <ArduinoJson.h>
#include <vector>

#include "base/core_hooks.h"
#include "core/BoardProfile.hpp"
#include "core/Calibration.hpp"
#include "core/ConfigManager.hpp"
#include "core/Measurement.hpp"
#include "core/SemaCore.hpp"
#include "core/Time.hpp"
#include "core/alarms/RuleEngine.hpp"
#include "core/derived/DerivedCalculator.hpp"
#include "core/prediction/Forecast.hpp"
#include "core/sensors/AdcSensor.hpp"
#include "core/sensors/Aht20Sensor.hpp"
#include "core/sensors/Bh1750Sensor.hpp"
#include "core/sensors/Bme280Sensor.hpp"
#include "core/sensors/Ds18b20Sensor.hpp"
#include "core/sensors/I2cScanner.hpp"
#include "core/sensors/SensorFactory.hpp"
#include "core/sensors/Sht40Sensor.hpp"

namespace sema {
namespace {

// ---------------------------------------------------------------------------
// Estado de dominio (la base ya no posee derived_ ni forecast_).
// ---------------------------------------------------------------------------
DerivedCalculator g_derived;
Forecast g_forecast;

// ---------------------------------------------------------------------------
// Catálogo de sensores por defecto (pines fijos del Board Profile).
// La base decide cuándo usarlo: aplica registerDefaultSensors solo cuando la
// configuración no trae sensores (config.sensors.empty()).
// ---------------------------------------------------------------------------
void registerDefaultSensors(base::SensorManager& sensors) {
  static Bme280Sensor bme280("EXT", SEMA_PIN_I2C_SDA, SEMA_PIN_I2C_SCL);
  static Sht40Sensor sht40("INT", SEMA_PIN_I2C_SDA, SEMA_PIN_I2C_SCL);
  static Ds18b20Sensor ds18b20("SOIL", SEMA_PIN_ONEWIRE, nullptr);
  static Bh1750Sensor bh1750("LUX", SEMA_PIN_I2C_SDA, SEMA_PIN_I2C_SCL);
  static Aht20Sensor aht20("AUX", SEMA_PIN_I2C_SDA, SEMA_PIN_I2C_SCL);
  // Batería (ADC interno): divisor 11:1 para 12 V.
  static AdcSensor battery("BATT", SEMA_PIN_BATTERY_ADC, "voltage", "V",
                           3.3f * 11.0f / 4095.0f, 0.0f);
  sensors.registerSensor(&bme280);
  sensors.registerSensor(&sht40);
  sensors.registerSensor(&ds18b20);
  sensors.registerSensor(&bh1750);
  sensors.registerSensor(&aht20);
  sensors.registerSensor(&battery);
}

void registerDefaultRules(base::RuleEngine& rules) {
  rules.addRule({"high_temp", "EXT", "temperature", base::RuleOp::Gt, 40.0f});
}

void registerDefaultCalibrations(base::SensorManager& sensors) {
  base::Calibration tempCal;
  tempCal.enabled = true;
  tempCal.gain = 1.0f;
  tempCal.offset = 0.0f;
  tempCal.hasRange = true;
  tempCal.min = -40.0f;
  tempCal.max = 85.0f;
  sensors.setCalibration("EXT:temperature", tempCal);
  sensors.setCalibration("INT:temperature", tempCal);
}

base::Sensor* createSensor(const base::SensorSpec& spec) {
  return SensorFactory::create(spec);
}

void derivedConfigure(const base::SystemConfig& sys) {
  g_derived.configure(sys);
}

// Las magnitudes derivadas se calculan sobre las mediciones crudas ya leídas y
// se anexan in-place (evita aliasing del vector de entrada).
void computeDerived(std::vector<base::Measurement>& measurements) {
  std::vector<base::Measurement> tmp;
  g_derived.compute(measurements, tmp, "metric");
  for (const base::Measurement& m : tmp) {
    measurements.push_back(m);
  }
}

void forecastAddSample(const char* measurement, uint32_t timestamp, float value) {
  g_forecast.addSample(measurement, timestamp, value);
}

void domainInit() {
  Wire.begin(SEMA_PIN_I2C_SDA, SEMA_PIN_I2C_SCL);
  std::vector<DetectedDevice> devices;
  I2cScanner::scan(devices);
  for (const DetectedDevice& d : devices) {
    Serial.printf("I²C 0x%02X → %s\n", d.address,
                  d.model.length() > 0 ? d.model.c_str() : "desconocido");
  }
}

// ---------------------------------------------------------------------------
// Web de dominio: catálogo TYPES[] y etiquetas MEAS{} (devuelven solo el
// literal JS, sin `const TYPES=`/`const MEAS=` ni `;`: la base los inyecta).
// ---------------------------------------------------------------------------
String sensorCatalogJson() {
  return String(R"js([
 {m:'BME280',name:'BME280',i:'i2c',d:'Temperatura, humedad y presión barométrica (I²C).'},
 {m:'BMP280',name:'BMP280',i:'i2c',d:'Presión barométrica y temperatura (I²C).'},
 {m:'SHT40',name:'SHT40',i:'i2c',d:'Temperatura y humedad (I²C).'},
 {m:'SHT31',name:'SHT31',i:'i2c',d:'Temperatura y humedad (I²C).'},
 {m:'AHT20',name:'AHT20',i:'i2c',d:'Temperatura y humedad (I²C).'},
 {m:'AHT10',name:'AHT10',i:'i2c',d:'Temperatura y humedad (I²C, addr 0x38).'},
 {m:'BH1750',name:'BH1750',i:'i2c',d:'Luz ambiente en lux (I²C).'},
 {m:'VEML6075',name:'VEML6075',i:'i2c',d:'Luz ultravioleta UVA y UVB (I²C).'},
 {m:'SCD30',name:'SCD30',i:'i2c',d:'CO₂, temperatura y humedad (I²C).'},
 {m:'SGP30',name:'SGP30',i:'i2c',d:'Calidad de aire: CO₂ y TVOC (I²C).'},
 {m:'ADS1115',name:'ADS1115',i:'i2c',d:'ADC externo 16 bits por I²C (4 canales A0-A3). Los canales se eligen desde cada sensor analógico.'},
 {m:'AS3935',name:'AS3935',i:'i2c',d:'Detector de rayos / relámpagos (I²C).'},
 {m:'DS18B20',name:'DS18B20',i:'1w',d:'Temperatura por bus 1-Wire (Dallas).',multi:true},
 {m:'ADC',name:'Veleta',i:'ana',ch:'wind_direction',d:'Dirección del viento (veleta, entrada analógica).'},
 {m:'ADC',name:'Batería',i:'ana',ch:'voltage',d:'Tensión de batería (entrada analógica).'},
 {m:'PCNT',name:'Anemómetro',i:'pulse',ch:'wind_speed',d:'Velocidad del viento (contador de pulsos).'},
 {m:'PCNT',name:'Pluviómetro',i:'pulse',ch:'rain',d:'Lluvia (contador de pulsos).'},
 {m:'PMS5003',name:'PMS5003',i:'uart',d:'Partículas PM1/PM2.5/PM10 (UART).'},
 {m:'CO',name:'CO',i:'ana',d:'Monóxido de carbono (analógico).'},
 {m:'SOLAR',name:'SOLAR',i:'ana',d:'Radiación solar (analógico).'},
 {m:'CCS811',name:'CCS811',i:'i2c',d:'Calidad de aire: CO₂ equivalente (eCO₂) y TVOC (I²C).'},
 {m:'HDC1080',name:'HDC1080',i:'i2c',d:'Temperatura y humedad (I²C).'},
 {m:'S12SD',name:'S12SD (UV)',i:'ana',d:'Radiación ultravioleta UVI (analógico).'}
])js");
}

String measurementLabelsJson() {
  return String(R"js({
 temperature:['Temperatura','Temperatura del aire'],humidity:['Humedad','Humedad relativa (%)'],pressure:['Presión','Presión barométrica (hPa)'],
 light:['Luz','Luminosidad (lux)'],uvi:['Índice UV','Radiación ultravioleta'],co2:['CO₂','Dióxido de carbono (ppm)'],
 pm25:['PM2.5','Partículas finas (2.5 µm)'],pm10:['PM10','Partículas (10 µm)'],wind_speed:['Viento','Velocidad del viento'],
 wind_gust:['Ráfagas','Ráfaga de viento'],wind_direction:['Dirección','Dirección del viento'],rain:['Lluvia','Precipitación'],
 voltage:['Batería','Tensión de batería (V)'],solar_radiation:['Solar','Radiación solar (W/m²)'],clock:['Reloj','Hora actual'],
 dew_point:['Rocío','Punto de rocío'],heat_index:['Índice calor','Sensación térmica por humedad'],wind_chill:['Sensación','Sensación térmica por viento'],
 qnh:['QNH','Presión a nivel del mar'],vpd:['VPD','Déficit de presión de vapor'],aqi:['AQI','Índice de calidad del aire'],
 barometric_altitude:['Altitud','Altitud barométrica'],rain_rate:['Lluvia/h','Tasa de lluvia (mm/h)'],rain_accumulated:['Lluvia ac.','Lluvia acumulada']
})js");
}

// /api/v1/sensors — catálogo + mediciones (crudas + derivadas + reloj), con
// conversión de unidades según el flag `imperial`.
String sensorsJson(const base::SemaCore& core, bool imperial) {
  base::SemaCore& c = const_cast<base::SemaCore&>(core);
  const String units = c.config().get().system.units;

#if SEMA_DEMO
  // Modo demo: valores ficticios dentro del rango estándar de cada magnitud.
  {
    DynamicJsonDocument ddoc(8192);
    JsonArray dcat = ddoc.createNestedArray("catalog");
    const char* models[] = {"BME280", "SHT40", "SCD30", "PMS5003", "VEML6075", "WH-SP-WD", "RG-9", "ESP32-ADC", "SOLAR"};
    const char* ids[] = {"ext", "int", "co2", "pm", "uv", "wind", "rain", "batt", "solar"};
    for (int i = 0; i < 9; ++i) {
      JsonObject o = dcat.createNestedObject();
      o["id"] = ids[i];
      o["model"] = models[i];
      o["interface"] = "demo";
      o["healthy"] = true;
    }
    const float t = millis() / 1000.0f;
    JsonArray darr = ddoc.createNestedArray("measurements");
    std::vector<base::Measurement> rawVec;
    auto wave = [&](float seed, float lo, float hi, float period) {
      return lo + (hi - lo) * (0.5f + 0.5f * sinf(t * 6.283185f / period + seed));
    };
    auto add = [&](const char* id, const char* meas, float v, const char* baseUnit) {
      base::Measurement rm;
      rm.sensorId = id;
      rm.channelId = "0";
      rm.measurement = meas;
      rm.value = v;
      rm.unit = baseUnit;
      rm.quality = base::Quality::Valid;
      rm.timestamp = base::nowEpoch();
      rawVec.push_back(rm);

      String u;
      const float cv = DerivedCalculator::convertUnit(v, meas, baseUnit, imperial, u);
      JsonObject o = darr.createNestedObject();
      o["sensor_id"] = id;
      o["channel_id"] = "0";
      o["measurement"] = meas;
      o["value"] = cv;
      o["unit"] = u;
      o["quality"] = "VALID";
      o["sequence"] = 1;
    };
    add("ext", "temperature", wave(0.0f, 18.0f, 28.0f, 3600), "°C");
    add("ext", "humidity", wave(1.0f, 45.0f, 75.0f, 5400), "%");
    add("ext", "pressure", wave(2.0f, 1008.0f, 1018.0f, 7200), "hPa");
    add("int", "temperature", wave(3.0f, 20.0f, 26.0f, 4200), "°C");
    add("int", "humidity", wave(4.0f, 40.0f, 65.0f, 4800), "%");
    add("uv", "uvi", wave(5.0f, 0.5f, 7.0f, 3000), "");
    add("uv", "light", wave(6.0f, 200.0f, 9000.0f, 2800), "lux");
    add("co2", "co2", wave(7.0f, 420.0f, 750.0f, 6000), "ppm");
    add("pm", "pm25", wave(8.0f, 8.0f, 25.0f, 5000), "µg/m³");
    add("pm", "pm10", wave(8.5f, 12.0f, 40.0f, 5000), "µg/m³");
    add("wind", "wind_speed", wave(9.0f, 2.0f, 12.0f, 2400), "m/s");
    add("wind", "wind_gust", wave(9.5f, 4.0f, 18.0f, 2400), "m/s");
    add("wind", "wind_direction", fmodf(t * 12.0f, 360.0f), "°");
    add("rain", "rain", wave(10.0f, 0.0f, 3.0f, 9000), "mm");
    add("batt", "voltage", wave(11.0f, 11.5f, 13.6f, 6000), "V");
    add("solar", "solar_radiation", wave(12.0f, 0.0f, 800.0f, 3600), "W/m²");
    add("clock", "clock", static_cast<float>(base::nowEpoch()), "epoch");

    std::vector<base::Measurement> derivedVec;
    g_derived.compute(rawVec, derivedVec, units);
    for (const base::Measurement& m : derivedVec) {
      JsonObject o = darr.createNestedObject();
      o["sensor_id"] = m.sensorId;
      o["channel_id"] = m.channelId;
      o["measurement"] = m.measurement;
      o["value"] = m.value;
      o["unit"] = m.unit;
      o["quality"] = base::qualityName(m.quality);
      o["sequence"] = 0;
    }

    ddoc["units"] = units;
    String dout;
    serializeJson(ddoc, dout);
    return dout;
  }
#endif

  DynamicJsonDocument doc(8192);

  JsonArray catalog = doc.createNestedArray("catalog");
  std::vector<base::SensorInfo> info;
  c.sensors().describe(info);
  for (const base::SensorInfo& s : info) {
    JsonObject o = catalog.createNestedObject();
    o["id"] = s.id;
    o["model"] = s.model;
    o["interface"] = s.interface;
    o["healthy"] = s.healthy;
  }

  std::vector<base::Measurement> derived;
  g_derived.compute(c.sensors().measurements(), derived, units);

  JsonArray arr = doc.createNestedArray("measurements");
  for (const base::Measurement& m : c.sensors().measurements()) {
    String u;
    const float v = DerivedCalculator::convertUnit(m.value, m.measurement,
                                                   m.unit, imperial, u);
    JsonObject o = arr.createNestedObject();
    o["sensor_id"] = m.sensorId;
    o["channel_id"] = m.channelId;
    o["measurement"] = m.measurement;
    o["value"] = v;
    o["unit"] = u;
    o["quality"] = base::qualityName(m.quality);
    o["sequence"] = m.sequence;
  }
  for (const base::Measurement& m : derived) {
    JsonObject o = arr.createNestedObject();
    o["sensor_id"] = m.sensorId;
    o["channel_id"] = m.channelId;
    o["measurement"] = m.measurement;
    o["value"] = m.value;
    o["unit"] = m.unit;
    o["quality"] = base::qualityName(m.quality);
    o["sequence"] = m.sequence;
  }

  {
    JsonObject o = arr.createNestedObject();
    o["sensor_id"] = "clock";
    o["channel_id"] = "0";
    o["measurement"] = "clock";
    o["value"] = static_cast<float>(base::nowEpoch());
    o["unit"] = "epoch";
    o["quality"] = "VALID";
    o["sequence"] = 0;
  }

  doc["units"] = units;

  String out;
  serializeJson(doc, out);
  return out;
}

// /api/v1/forecast — predicciones temperatura/humedad/lluvia a 3/6/12/24 h.
String forecastJson() {
  DynamicJsonDocument doc(1024);
  JsonArray arr = doc.createNestedArray("predictions");
  const char* mags[] = {"temperature", "humidity", "rain"};
  const uint32_t horizons[] = {3, 6, 12, 24};
  for (const char* m : mags) {
    JsonObject o = arr.createNestedObject();
    o["measurement"] = m;
    JsonArray hz = o.createNestedArray("horizons");
    for (uint32_t h : horizons) {
      float value = 0.0f, confidence = 0.0f;
      const bool ok = g_forecast.predict(m, h, value, confidence);
      JsonObject p = hz.createNestedObject();
      p["hours"] = h;
      p["available"] = ok;
      p["value"] = value;
      p["confidence"] = confidence;
    }
  }
  String out;
  serializeJson(doc, out);
  return out;
}

// /api/v1/wind/north — auto-calibración del norte de la veleta.
// La base ya chequea auth antes de llamar al hook.
String windNorthJson(const base::SemaCore& core, int& status) {
  base::SemaCore& c = const_cast<base::SemaCore&>(core);
  const base::SystemConfig& sys = c.config().get().system;
  if (sys.weather.windDirectionPin == 0) {
    status = 400;
    return "{\"error\":\"wind direction not configured\"}";
  }
  const uint16_t adc = analogRead(sys.weather.windDirectionPin);
  const float rawAngle = DerivedCalculator::windVaneRawAngle(adc, sys);
  base::Config next = c.config().get();
  next.system.weather.windNorthOffset = rawAngle;
  if (!c.config().apply(next)) {
    status = 500;
    return "{\"error\":\"config apply failed\"}";
  }
  g_derived.configure(c.config().get().system);
  status = 200;
  return "{\"ok\":true}";
}

// /api/v1/wind/resistors — red de resistencias de la veleta.
// La base ya chequea auth y presencia de body antes de llamar al hook.
String windResistorsJson(const base::SemaCore& core, const String& body, int& status) {
  base::SemaCore& c = const_cast<base::SemaCore&>(core);
  DynamicJsonDocument doc(512);
  if (deserializeJson(doc, body)) {
    status = 400;
    return "{\"error\":\"invalid json\"}";
  }
  base::Config next = c.config().get();
  next.system.weather.windRpull = doc["rpull"] | 10000.0f;
  JsonArray arr = doc["resistors"].as<JsonArray>();
  for (uint8_t i = 0; i < 8 && i < arr.size(); ++i) {
    next.system.weather.windResistors[i] = arr[i].as<float>();
  }
  if (!c.config().apply(next)) {
    status = 500;
    return "{\"error\":\"config apply failed\"}";
  }
  g_derived.configure(c.config().get().system);
  status = 200;
  return "{\"ok\":true}";
}

}  // namespace

void registerDomainHooks() {
  base::CoreHooks& h = base::coreHooks();
  h.createSensor = createSensor;
  h.registerDefaultSensors = registerDefaultSensors;
  h.registerDefaultRules = registerDefaultRules;
  h.registerDefaultCalibrations = registerDefaultCalibrations;
  h.derivedConfigure = derivedConfigure;
  h.computeDerived = computeDerived;
  h.forecastAddSample = forecastAddSample;
  h.domainInit = domainInit;
  h.sensorCatalogJson = sensorCatalogJson;
  h.measurementLabelsJson = measurementLabelsJson;
  h.sensorsJson = sensorsJson;
  h.forecastJson = forecastJson;
  h.windNorthJson = windNorthJson;
  h.windResistorsJson = windResistorsJson;
}

}  // namespace sema
