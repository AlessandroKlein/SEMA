#include "core/ConfigManager.hpp"

#include <ArduinoJson.h>

#include "hw/HwProfile.hpp"

namespace sema {

static const char* kConfigKey = "config";

// Si SEMA_PINS_FROM_FILE, fija los pines desde HwProfile.hpp (PCB) e ignora
// los que vengan de la web.
static void applyHwProfile(Config& c) {
#if SEMA_PINS_FROM_FILE
  c.can.txPin = SEMA_PIN_CAN_TX;
  c.can.rxPin = SEMA_PIN_CAN_RX;

  c.modbus.rxPin = SEMA_PIN_MODBUS_RX;
  c.modbus.txPin = SEMA_PIN_MODBUS_TX;
  c.modbus.deRePin = SEMA_PIN_MODBUS_DERE;

  c.zigbee.rxPin = SEMA_PIN_ZIGBEE_RX;
  c.zigbee.txPin = SEMA_PIN_ZIGBEE_TX;

  c.lora.csPin = SEMA_CS_LORA;
  c.lora.rstPin = SEMA_PIN_LORA_RST;
  c.lora.dio1Pin = SEMA_PIN_LORA_DIO1;
  c.lora.busyPin = SEMA_PIN_LORA_BUSY;

  c.ethernet.mdcPin = SEMA_PIN_ETH_MDC;
  c.ethernet.mdioPin = SEMA_PIN_ETH_MDIO;
  c.ethernet.phyAddr = SEMA_PIN_ETH_PHY_ADDR;
  c.ethernet.powerPin = SEMA_PIN_ETH_POWER;
  c.ethernet.csPin = SEMA_CS_ETHERNET_W5500;
  c.ethernet.rstPin = SEMA_PIN_ETH_W5500_RST;
  c.ethernet.irqPin = SEMA_PIN_ETH_W5500_IRQ;
  c.ethernet.sckPin = SEMA_SPI_SCK;
  c.ethernet.misoPin = SEMA_SPI_MISO;
  c.ethernet.mosiPin = SEMA_SPI_MOSI;
#endif
}

ConfigManager::ConfigManager(KeyValueStore& store) : store_(store) {}

bool ConfigManager::load() {
  String raw;
  if (store_.getString(kConfigKey, raw) && raw.length() > 0) {
    if (deserialize(raw) && validate(config_)) {
      valid_ = true;
      return true;
    }
  }

  // Defaults (schema=1).
  config_ = Config{};
  config_.schemaVersion = 1;
  config_.station.id = "SEMA-001";
  config_.station.name = "Estación Norte";
  config_.network.mode = "STA";
  config_.network.hostname = "sema-001";
  config_.network.mdns = true;
  config_.system.timezone = "America/Argentina/Buenos_Aires";
  config_.system.logLevel = "INFO";
  config_.storage.backend = "littlefs";
  config_.storage.retentionDays = 30;

  valid_ = validate(config_);
  return valid_;
}

bool ConfigManager::save() {
  String raw;
  if (!serialize(raw)) {
    Serial.println("[cfg] serialize failed");
    return false;
  }
  const bool ok = store_.putString(kConfigKey, raw.c_str());
  if (!ok) {
    Serial.printf("[cfg] putString failed (len=%u)\n", static_cast<unsigned>(raw.length()));
  }
  return ok;
}

bool ConfigManager::apply(const Config& next) {
  if (!validate(next)) {
    return false;
  }
  backup_ = config_;
  config_ = next;
  if (!save()) {
    config_ = backup_;  // rollback
    return false;
  }
  valid_ = true;
  return true;
}

bool ConfigManager::validate(const Config& c) const {
  if (c.schemaVersion != 1) {
    return false;
  }
  if (c.station.id.length() == 0) {
    return false;
  }
  if (c.network.mode != "STA" && c.network.mode != "AP") {
    return false;
  }
  if (c.storage.backend != "littlefs" && c.storage.backend != "flash" &&
      c.storage.backend != "sd") {
    return false;
  }
  return true;
}

bool ConfigManager::serialize(String& out) const {
  DynamicJsonDocument doc(16384);
  doc["schema_version"] = config_.schemaVersion;
  doc["station"]["id"] = config_.station.id;
  doc["station"]["name"] = config_.station.name;
  doc["network"]["mode"] = config_.network.mode;
  doc["network"]["ssid"] = config_.network.ssid;
  doc["network"]["password"] = config_.network.password;
  doc["network"]["hostname"] = config_.network.hostname;
  doc["network"]["mdns"] = config_.network.mdns;
  doc["network"]["ip"] = config_.network.ip;
  doc["network"]["gateway"] = config_.network.gateway;
  doc["network"]["subnet"] = config_.network.subnet;
  doc["network"]["dns"] = config_.network.dns;
  doc["system"]["timezone"] = config_.system.timezone;
  doc["system"]["ntp_server"] = config_.system.ntpServer;
  doc["system"]["log_level"] = config_.system.logLevel;
  doc["system"]["units"] = config_.system.units;
  doc["system"]["altitude"] = config_.system.altitude;
  doc["system"]["wind_north_offset"] = config_.system.windNorthOffset;
  doc["system"]["wind_direction_pin"] = config_.system.windDirectionPin;
  doc["system"]["wind_rpull"] = config_.system.windRpull;
  JsonArray wr = doc["system"].createNestedArray("wind_resistors");
  for (uint8_t i = 0; i < 8; ++i) {
    wr.add(config_.system.windResistors[i]);
  }

  doc["storage"]["backend"] = config_.storage.backend;
  doc["storage"]["retention_days"] = config_.storage.retentionDays;
  doc["security"]["api_key"] = config_.security.apiKey;
  doc["security"]["server_key"] = config_.security.serverKey;
  doc["security"]["username"] = config_.security.username;
  doc["security"]["password"] = config_.security.password;
  doc["security"]["extra_keys"] = config_.security.extraKeys;
  doc["energy"]["rain_pin"] = config_.energy.rainPin;
  doc["publishers"]["webhook_url"] = config_.publishers.webhookUrl;
  doc["publishers"]["mqtt_host"] = config_.publishers.mqttHost;
  doc["publishers"]["mqtt_port"] = config_.publishers.mqttPort;
  doc["publishers"]["mqtt_topic"] = config_.publishers.mqttTopic;
  doc["publishers"]["mqtt_user"] = config_.publishers.mqttUser;
  doc["publishers"]["mqtt_pass"] = config_.publishers.mqttPass;
  JsonArray rules = doc.createNestedArray("rules");
  for (const RuleSpec& r : config_.rules) {
    JsonObject o = rules.createNestedObject();
    o["name"] = r.name;
    o["sensor_id"] = r.sensorId;
    o["channel_id"] = r.channelId;
    o["op"] = r.op;
    o["value"] = r.value;
  }
  JsonArray calibrations = doc.createNestedArray("calibrations");
  for (const CalibrationSpec& c : config_.calibrations) {
    JsonObject o = calibrations.createNestedObject();
    o["sensor_id"] = c.sensorId;
    o["channel_id"] = c.channelId;
    o["gain"] = c.gain;
    o["offset"] = c.offset;
    o["has_range"] = c.hasRange;
    o["min"] = c.min;
    o["max"] = c.max;
  }
  JsonArray sensors = doc.createNestedArray("sensors");
  for (const SensorSpec& s : config_.sensors) {
    JsonObject o = sensors.createNestedObject();
    o["id"] = s.id;
    o["model"] = s.model;
    o["enabled"] = s.enabled;
    o["address"] = s.address;
    o["rom"] = s.rom;
    o["sda"] = s.sda;
    o["scl"] = s.scl;
    o["pin"] = s.pin;
    o["rx"] = s.rxPin;
    o["tx"] = s.txPin;
    o["channel"] = s.channel;
    o["unit"] = s.unit;
    o["scale"] = s.scale;
    o["offset"] = s.offset;
  }
  JsonArray gpio = doc.createNestedArray("gpio");
  for (const GpioSpec& g : config_.gpio) {
    JsonObject o = gpio.createNestedObject();
    o["id"] = g.id;
    o["pin"] = g.pin;
    o["mode"] = g.mode;
    o["initial"] = g.initial;
    o["expander_addr"] = g.expanderAddr;
  }
  doc["mcp23017_addr"] = config_.mcp23017Addr;
  doc["shift_register"]["type"] = config_.shiftRegister.type;
  doc["shift_register"]["data_pin"] = config_.shiftRegister.dataPin;
  doc["shift_register"]["clock_pin"] = config_.shiftRegister.clockPin;
  doc["shift_register"]["latch_pin"] = config_.shiftRegister.latchPin;
  doc["modbus"]["enabled"] = config_.modbus.enabled;
  doc["modbus"]["rx"] = config_.modbus.rxPin;
  doc["modbus"]["tx"] = config_.modbus.txPin;
  doc["modbus"]["de_re"] = config_.modbus.deRePin;
  doc["modbus"]["baud"] = config_.modbus.baud;
  doc["modbus"]["slave_id"] = config_.modbus.slaveId;
  doc["modbus"]["register"] = config_.modbus.registerAddr;
  doc["modbus"]["count"] = config_.modbus.registerCount;
  doc["can"]["enabled"] = config_.can.enabled;
  doc["can"]["tx"] = config_.can.txPin;
  doc["can"]["rx"] = config_.can.rxPin;
  doc["can"]["speed"] = config_.can.speed;
  doc["lora"]["enabled"] = config_.lora.enabled;
  doc["lora"]["cs"] = config_.lora.csPin;
  doc["lora"]["rst"] = config_.lora.rstPin;
  doc["lora"]["dio1"] = config_.lora.dio1Pin;
  doc["lora"]["busy"] = config_.lora.busyPin;
  doc["lora"]["frequency"] = config_.lora.frequency;
  doc["lora"]["bandwidth"] = config_.lora.bandwidth;
  doc["lora"]["spreading"] = config_.lora.spreading;
  doc["lora"]["coding_rate"] = config_.lora.codingRate;
  doc["lora"]["tx_power"] = config_.lora.txPower;
  doc["zigbee"]["enabled"] = config_.zigbee.enabled;
  doc["zigbee"]["rx"] = config_.zigbee.rxPin;
  doc["zigbee"]["tx"] = config_.zigbee.txPin;
  doc["zigbee"]["baud"] = config_.zigbee.baud;
  doc["ethernet"]["enabled"] = config_.ethernet.enabled;
  doc["ethernet"]["mdc"] = config_.ethernet.mdcPin;
  doc["ethernet"]["mdio"] = config_.ethernet.mdioPin;
  doc["ethernet"]["phy_addr"] = config_.ethernet.phyAddr;
  doc["ethernet"]["power"] = config_.ethernet.powerPin;
  doc["ethernet"]["cs"] = config_.ethernet.csPin;
  doc["ethernet"]["rst"] = config_.ethernet.rstPin;
  doc["ethernet"]["irq"] = config_.ethernet.irqPin;
  doc["ethernet"]["sck"] = config_.ethernet.sckPin;
  doc["ethernet"]["miso"] = config_.ethernet.misoPin;
  doc["ethernet"]["mosi"] = config_.ethernet.mosiPin;
  serializeJson(doc, out);
  return true;
}

bool ConfigManager::parseInto(const String& in, Config& c) {
  DynamicJsonDocument doc(16384);
  const DeserializationError err = deserializeJson(doc, in);
  if (err) {
    return false;
  }
  c.schemaVersion = doc["schema_version"] | 1;
  c.station.id = doc["station"]["id"] | "SEMA-001";
  c.station.name = doc["station"]["name"] | "Estación Norte";
  c.network.mode = doc["network"]["mode"] | "STA";
  c.network.ssid = doc["network"]["ssid"] | "";
  c.network.password = doc["network"]["password"] | "";
  c.network.hostname = doc["network"]["hostname"] | "sema-001";
  c.network.mdns = doc["network"]["mdns"] | true;
  c.network.ip = doc["network"]["ip"] | "";
  c.network.gateway = doc["network"]["gateway"] | "";
  c.network.subnet = doc["network"]["subnet"] | "";
  c.network.dns = doc["network"]["dns"] | "";
  c.system.timezone = doc["system"]["timezone"] | "America/Argentina/Buenos_Aires";
  c.system.ntpServer = doc["system"]["ntp_server"] | "pool.ntp.org";
  c.system.logLevel = doc["system"]["log_level"] | "INFO";
  c.system.units = doc["system"]["units"] | "metric";
  c.system.altitude = doc["system"]["altitude"] | 0.0f;
  c.system.windNorthOffset = doc["system"]["wind_north_offset"] | 0.0f;
  c.system.windDirectionPin = doc["system"]["wind_direction_pin"] | 0;
  c.system.windRpull = doc["system"]["wind_rpull"] | 10000.0f;
  JsonArray wr = doc["system"]["wind_resistors"].as<JsonArray>();
  for (uint8_t i = 0; i < 8 && i < wr.size(); ++i) {
    c.system.windResistors[i] = wr[i] | 0.0f;
  }

  c.storage.backend = doc["storage"]["backend"] | "littlefs";
  c.storage.retentionDays = doc["storage"]["retention_days"] | 30;
  c.security.apiKey = doc["security"]["api_key"] | "";
  c.security.serverKey = doc["security"]["server_key"] | "";
  c.security.username = doc["security"]["username"] | "";
  c.security.password = doc["security"]["password"] | "";
  c.security.extraKeys = doc["security"]["extra_keys"] | "";
  c.energy.rainPin = doc["energy"]["rain_pin"] | 0;
  c.publishers.webhookUrl = doc["publishers"]["webhook_url"] | "";
  c.publishers.mqttHost = doc["publishers"]["mqtt_host"] | "";
  c.publishers.mqttPort = doc["publishers"]["mqtt_port"] | 1883;
  c.publishers.mqttTopic = doc["publishers"]["mqtt_topic"] | "sema/measurement";
  c.publishers.mqttUser = doc["publishers"]["mqtt_user"] | "";
  c.publishers.mqttPass = doc["publishers"]["mqtt_pass"] | "";
  c.rules.clear();
  for (JsonObject o : doc["rules"].as<JsonArray>()) {
    RuleSpec r;
    r.name = o["name"] | "";
    r.sensorId = o["sensor_id"] | "";
    r.channelId = o["channel_id"] | "";
    r.op = o["op"] | "gt";
    r.value = o["value"] | 0.0f;
    c.rules.push_back(r);
  }
  c.calibrations.clear();
  for (JsonObject o : doc["calibrations"].as<JsonArray>()) {
    CalibrationSpec cal;
    cal.sensorId = o["sensor_id"] | "";
    cal.channelId = o["channel_id"] | "";
    cal.gain = o["gain"] | 1.0f;
    cal.offset = o["offset"] | 0.0f;
    cal.hasRange = o["has_range"] | false;
    cal.min = o["min"] | 0.0f;
    cal.max = o["max"] | 0.0f;
    c.calibrations.push_back(cal);
  }
  c.sensors.clear();
  for (JsonObject o : doc["sensors"].as<JsonArray>()) {
    SensorSpec s;
    s.id = o["id"] | "";
    s.model = o["model"] | "";
    s.enabled = o["enabled"] | false;
    s.address = o["address"] | 0;
    s.rom = o["rom"] | "";
    s.sda = o["sda"] | 21;
    s.scl = o["scl"] | 22;
    s.pin = o["pin"] | 0;
    s.rxPin = o["rx"] | 0;
    s.txPin = o["tx"] | 0;
    s.channel = o["channel"] | "";
    s.unit = o["unit"] | "";
    s.scale = o["scale"] | 1.0f;
    s.offset = o["offset"] | 0.0f;
    c.sensors.push_back(s);
  }
  c.gpio.clear();
  for (JsonObject o : doc["gpio"].as<JsonArray>()) {
    GpioSpec g;
    g.id = o["id"] | "";
    g.pin = o["pin"] | 0;
    g.mode = o["mode"] | "input";
    g.initial = o["initial"] | 0;
    g.expanderAddr = o["expander_addr"] | 0;
    c.gpio.push_back(g);
  }
  c.mcp23017Addr = doc["mcp23017_addr"] | 0;
  c.shiftRegister.type = doc["shift_register"]["type"] | "74HC595";
  c.shiftRegister.dataPin = doc["shift_register"]["data_pin"] | 0;
  c.shiftRegister.clockPin = doc["shift_register"]["clock_pin"] | 0;
  c.shiftRegister.latchPin = doc["shift_register"]["latch_pin"] | 0;
  c.modbus.enabled = doc["modbus"]["enabled"] | false;
  c.modbus.rxPin = doc["modbus"]["rx"] | 16;
  c.modbus.txPin = doc["modbus"]["tx"] | 17;
  c.modbus.deRePin = doc["modbus"]["de_re"] | 0;
  c.modbus.baud = doc["modbus"]["baud"] | 9600;
  c.modbus.slaveId = doc["modbus"]["slave_id"] | 1;
  c.modbus.registerAddr = doc["modbus"]["register"] | 0;
  c.modbus.registerCount = doc["modbus"]["count"] | 4;
  c.can.enabled = doc["can"]["enabled"] | false;
  c.can.txPin = doc["can"]["tx"] | 5;
  c.can.rxPin = doc["can"]["rx"] | 4;
  c.can.speed = doc["can"]["speed"] | 500000;
  c.lora.enabled = doc["lora"]["enabled"] | false;
  c.lora.csPin = doc["lora"]["cs"] | 5;
  c.lora.rstPin = doc["lora"]["rst"] | 14;
  c.lora.dio1Pin = doc["lora"]["dio1"] | 26;
  c.lora.busyPin = doc["lora"]["busy"] | 27;
  c.lora.frequency = doc["lora"]["frequency"] | 915.0f;
  c.lora.bandwidth = doc["lora"]["bandwidth"] | 125.0f;
  c.lora.spreading = doc["lora"]["spreading"] | 7;
  c.lora.codingRate = doc["lora"]["coding_rate"] | 5;
  c.lora.txPower = doc["lora"]["tx_power"] | 14;
  c.zigbee.enabled = doc["zigbee"]["enabled"] | false;
  c.zigbee.rxPin = doc["zigbee"]["rx"] | 16;
  c.zigbee.txPin = doc["zigbee"]["tx"] | 17;
  c.zigbee.baud = doc["zigbee"]["baud"] | 115200;
  c.ethernet.enabled = doc["ethernet"]["enabled"] | false;
  c.ethernet.mdcPin = doc["ethernet"]["mdc"] | 23;
  c.ethernet.mdioPin = doc["ethernet"]["mdio"] | 18;
  c.ethernet.phyAddr = doc["ethernet"]["phy_addr"] | 1;
  c.ethernet.powerPin = doc["ethernet"]["power"] | -1;
  c.ethernet.csPin = doc["ethernet"]["cs"] | 5;
  c.ethernet.rstPin = doc["ethernet"]["rst"] | -1;
  c.ethernet.irqPin = doc["ethernet"]["irq"] | 4;
  c.ethernet.sckPin = doc["ethernet"]["sck"] | 18;
  c.ethernet.misoPin = doc["ethernet"]["miso"] | 19;
  c.ethernet.mosiPin = doc["ethernet"]["mosi"] | 21;
  applyHwProfile(c);
  return true;
}

bool ConfigManager::deserialize(const String& in) {
  return parseInto(in, config_);
}

bool ConfigManager::applyJson(const String& json) {
  Config next;
  if (!parseInto(json, next)) {
    return false;
  }
  return apply(next);
}

bool ConfigManager::saveDashboardLayout(const String& layout) {
  return store_.putString("layout", layout.c_str());
}

bool ConfigManager::loadDashboardLayout(String& out) {
  return store_.getString("layout", out);
}

}  // namespace sema
