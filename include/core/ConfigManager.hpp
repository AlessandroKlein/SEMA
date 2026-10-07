#pragma once

#include <Arduino.h>
#include <cstdint>
#include <vector>

#include "core/storage/Storage.hpp"

// =============================================================================
// SEMA — ConfigManager (configuración transaccional, schema=1)
// =============================================================================
// D-0042 / D-0023 / D-0024. Configuración jerárquica versionada, persistida en
// NVS a través del Storage API. Aplica con validación previa y rollback.

namespace sema {

struct StationConfig {
  String id;
  String name;
};

struct NetworkConfig {
  String mode;      // "STA" | "AP"
  String ssid;
  String password;
  String hostname;
  bool mdns;
};

struct SystemConfig {
  String timezone;
  String logLevel;
  String units = "metric";          // "metric" | "imperial"
  float altitude = 0.0f;            // metros (para QNH y altitud barométrica)
  float windNorthOffset = 0.0f;     // grados (fine-tune del norte de la veleta)
  uint8_t windDirectionPin = 0;     // ADC de la veleta WH-SP-WD (0 = sin veleta)
  float windRpull = 10000.0f;       // pull-up de la veleta (Ω)
  // Red de 8 resistencias de la veleta, en orden del datasheet:
  // N, NE, E, SE, S, SO, O, NO. Las 16 posiciones son 8 directas + 8 en paralelo.
  float windResistors[8] = {33000.0f, 8200.0f, 1000.0f, 2200.0f,
                            3900.0f, 16000.0f, 120000.0f, 64900.0f};
  String dashboardLayout;  // JSON del layout Gridstack (global, no por usuario)
};

struct StorageConfig {
  String backend;   // "littlefs" | "flash" | "sd"
  uint32_t retentionDays;
};

struct SecurityConfig {
  String apiKey;     // clave de la web local (D-0048); vacía = sin autenticación
  String serverKey;  // clave del Servidor Central → SEMA (config de riesgo, vía API)
  String username;   // usuario del login web (vacío = "admin")
  String password;   // contraseña del login web (vacío = sin login)
};

struct EnergyConfig {
  uint8_t rainPin = 0;  // GPIO del pluviómetro (D-0022); 0 = deshabilitado
};

struct PublishersConfig {
  String webhookUrl;   // vacío = deshabilitado
  String mqttHost;     // vacío = deshabilitado
  uint16_t mqttPort = 1883;
  String mqttTopic = "sema/measurement";
  String mqttUser;     // vacío = sin autenticación
  String mqttPass;
};

// Especificación de una regla de alarma (D-0059): configurable.
struct RuleSpec {
  String name;
  String sensorId;   // "" = cualquier sensor
  String channelId;  // magnitud a vigilar
  String op;         // "gt" | "lt" | "ge" | "le"
  float value = 0.0f;
};

// Especificación de calibración por canal (D-0055): configurable.
struct CalibrationSpec {
  String sensorId;
  String channelId;
  float gain = 1.0f;
  float offset = 0.0f;
  bool hasRange = false;
  float min = 0.0f;
  float max = 0.0f;
};

// Especificación de un sensor (D-0042): el catálogo se define por configuración.
struct SensorSpec {
  String id;
  String model;      // "BME280" | "SHT40" | "DS18B20" | "BH1750" | "AHT20" | "ADC" | …
  bool enabled = true;  // false = sensor apagado (no se lee ni se deriva)
  uint8_t sda = 21;
  uint8_t scl = 22;
  uint8_t pin = 0;
  uint8_t rxPin = 0;  // para sensores UART (p. ej. PMS5003)
  uint8_t txPin = 0;
  String channel;    // para sensores analógicos (p. ej. "voltage")
  String unit;       // unidad (p. ej. "V")
  float scale = 1.0f;
  float offset = 0.0f;
};

// Especificación de un pin GPIO standalone (entradas/salidas digitales).
struct GpioSpec {
  String id;
  uint8_t pin = 0;
  String mode;      // "output" | "input" | "input_pullup" | "input_pulldown"
  uint8_t initial = 0;
  uint8_t expanderAddr = 0;  // 0 = pin nativo; != 0 = MCP23017 en esa dirección I²C
};

// Shift register standalone (74HC595 salida / 74HC165 entrada).
struct ShiftRegisterConfig {
  String type = "74HC595";  // "74HC595" (salida) | "74HC165" (entrada)
  uint8_t dataPin = 0;      // SER (595) / QH (165)
  uint8_t clockPin = 0;     // SRCLK (595) / CLK (165)
  uint8_t latchPin = 0;     // RCLK (595) / SH-LD (165)
};

// RS485 / Modbus RTU (maestro de un esclavo).
struct ModbusConfig {
  bool enabled = false;
  uint8_t rxPin = 16;
  uint8_t txPin = 17;
  uint8_t deRePin = 0;        // DE/RE del transceiver RS485 (0 = sin control)
  uint32_t baud = 9600;
  uint8_t slaveId = 1;
  uint16_t registerAddr = 0;  // dirección del primer registro
  uint16_t registerCount = 4; // cantidad de registros a leer
};

// CAN (TWAI, CAN 2.0).
struct CanConfig {
  bool enabled = false;
  uint8_t txPin = 5;
  uint8_t rxPin = 4;
  uint32_t speed = 500000;  // bps: 125000 | 250000 | 500000 | 1000000
};

// LoRa (SX1262, módulo Silicontra SX1262PATR8-GC).
struct LoraConfig {
  bool enabled = false;
  uint8_t csPin = 5;
  uint8_t rstPin = 14;
  uint8_t dio1Pin = 26;
  uint8_t busyPin = 27;
  float frequency = 915.0f;   // MHz
  float bandwidth = 125.0f;   // kHz
  uint8_t spreading = 7;      // 7..12
  uint8_t codingRate = 5;     // 5..8
  int8_t txPower = 14;        // dBm
};

// Zigbee (RF-BM-2652P2 / CC2652P2, ZNP por UART).
struct ZigbeeConfig {
  bool enabled = false;
  uint8_t rxPin = 16;
  uint8_t txPin = 17;
  uint32_t baud = 115200;
};

// Ethernet: nativo (LAN8720A por RMII) o SPI (W5500), según la board.
struct EthernetConfig {
  bool enabled = false;
  // LAN8720A (RMII) — board con MAC Ethernet nativa
  uint8_t mdcPin = 23;
  uint8_t mdioPin = 18;
  uint8_t phyAddr = 1;
  int powerPin = -1;   // -1 = sin control de alimentación de la PHY
  // W5500 (SPI) — board sin MAC nativa (driver ESP-IDF)
  uint8_t csPin = 5;
  int rstPin = -1;     // -1 = sin pin de reset
  int irqPin = 4;      // -1 = sin IRQ (modo polling)
  uint8_t sckPin = 18;
  uint8_t misoPin = 19;
  uint8_t mosiPin = 21;
};

// Configuración completa (schema=1).
struct Config {
  uint32_t schemaVersion = 1;
  StationConfig station;
  NetworkConfig network;
  SystemConfig system;
  StorageConfig storage;
  SecurityConfig security;
  EnergyConfig energy;
  PublishersConfig publishers;
  std::vector<SensorSpec> sensors;          // vacío = usar catálogo por defecto
  std::vector<RuleSpec> rules;              // vacío = usar regla por defecto
  std::vector<CalibrationSpec> calibrations; // vacío = usar calibración por defecto
  std::vector<GpioSpec> gpio;              // GPIO standalone (opcional)
  ShiftRegisterConfig shiftRegister;       // shift register (opcional)
  ModbusConfig modbus;                     // RS485/Modbus (opcional)
  CanConfig can;                           // CAN/TWAI (opcional)
  LoraConfig lora;                         // LoRa/SX1262 (opcional)
  ZigbeeConfig zigbee;                     // Zigbee/CC2652P2 (opcional)
  EthernetConfig ethernet;                 // Ethernet LAN8720A/W5500 (opcional)
};

class ConfigManager {
public:
  explicit ConfigManager(KeyValueStore& store);

  bool load();                    // carga desde NVS o aplica defaults
  bool save();                    // persiste el estado actual
  bool apply(const Config& next); // valida + aplica + persiste + rollback

  const Config& get() const { return config_; }
  bool valid() const { return valid_; }
  bool toJson(String& out) const { return serialize(out); }
  bool applyJson(const String& json);  // parsea y aplica transaccionalmente

private:
  bool validate(const Config& c) const;
  bool serialize(String& out) const;
  bool parseInto(const String& in, Config& c);
  bool deserialize(const String& in);

  KeyValueStore& store_;
  Config config_;
  Config backup_;
  bool valid_ = false;
};

}  // namespace sema
