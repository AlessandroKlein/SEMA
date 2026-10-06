#include "core/ZigbeeManager.hpp"

#include "hw/HwProfile.hpp"

#if SEMA_USE_ZIGBEE
#include <Arduino.h>
#include <string.h>

namespace sema {

void ZigbeeManager::apply(const ZigbeeConfig& cfg) {
  cfg_ = cfg;
  ready_ = false;
  rxPos_ = 0;
  hasMessage_ = false;
  if (!cfg_.enabled) {
    return;
  }

  Serial1.begin(cfg_.baud, SERIAL_8N1, cfg_.rxPin, cfg_.txPin);
  ready_ = true;

  // SYS_RESET (0x4100): soft reset del CC2652P2.
  const uint8_t type = 0x01;
  sendFrame(0x41, 0x00, &type, 1);
}

void ZigbeeManager::sendFrame(uint8_t cmd0, uint8_t cmd1, const uint8_t* payload,
                              uint8_t len) {
  if (!ready_) {
    return;
  }
  const uint8_t frameLen = len + 2;  // CMD0 + CMD1 + payload
  uint8_t fcs = frameLen;
  Serial1.write(0xFE);  // SOF
  Serial1.write(frameLen);
  Serial1.write(cmd0);
  Serial1.write(cmd1);
  fcs ^= cmd0;
  fcs ^= cmd1;
  if (payload != nullptr && len > 0) {
    for (uint8_t i = 0; i < len; ++i) {
      Serial1.write(payload[i]);
      fcs ^= payload[i];
    }
  }
  Serial1.write(fcs);
}

bool ZigbeeManager::send(uint16_t destination, const uint8_t* data, uint8_t len) {
  if (!ready_ || data == nullptr || len == 0 || len > 110) {
    return false;
  }

  // AF_DATA_REQUEST (0x2401).
  uint8_t payload[128];
  uint8_t p = 0;
  payload[p++] = destination & 0xFF;   // destination (uint16 LE)
  payload[p++] = (destination >> 8) & 0xFF;
  payload[p++] = 0x01;                 // destination endpoint
  payload[p++] = 0x01;                 // source endpoint
  payload[p++] = 0x01;                 // cluster id (uint16 LE, low)
  payload[p++] = 0x00;                 // cluster id (high)
  payload[p++] = transId_++;           // transaction id
  payload[p++] = 0x00;                 // options
  payload[p++] = 10;                   // radius
  payload[p++] = len;                  // data length
  memcpy(&payload[p], data, len);
  p += len;

  sendFrame(0x24, 0x01, payload, p);
  return true;
}

void ZigbeeManager::loop() {
  if (!ready_) {
    return;
  }
  while (Serial1.available() > 0) {
    const uint8_t b = Serial1.read();
    if (rxPos_ < sizeof(rxBuf_)) {
      rxBuf_[rxPos_++] = b;
    } else {
      rxPos_ = 0;  // buffer lleno, reinicia
    }

    // Una trama ZNP: SOF(1) LEN(1) CMD0(1) CMD1(1) payload(LEN-2) FCS(1).
    if (rxPos_ >= 4) {
      const uint8_t frameLen = rxBuf_[1];
      if (frameLen <= (sizeof(rxBuf_) - 4) &&
          rxPos_ >= static_cast<uint16_t>(frameLen + 4)) {
        const uint8_t cmd0 = rxBuf_[2];
        const uint8_t cmd1 = rxBuf_[3];
        handleFrame(cmd0, cmd1, &rxBuf_[4], frameLen - 2);
        rxPos_ = 0;
      }
    }
  }
}

void ZigbeeManager::handleFrame(uint8_t cmd0, uint8_t cmd1, const uint8_t* payload,
                                uint8_t len) {
  // AF_INCOMING_MSG (AREQ 0x4481).
  // payload: groupId(2) clusterId(2) srcAddr(2) srcEp(1) dstEp(1) wasBroadcast(1)
  //          linkQuality(1) securityUse(1) timestamp(4) transSeq(1) len(1) data(len)
  if (cmd0 == 0x44 && cmd1 == 0x81 && len >= 17) {
    lastSrc_ = payload[4] | (payload[5] << 8);  // srcAddr (offset 4)
    const uint8_t dataLen = payload[16];        // campo len (offset 16)
    if (dataLen <= sizeof(lastMsg_) && (len - 17) >= dataLen) {
      memcpy(lastMsg_, &payload[17], dataLen);
      lastLen_ = dataLen;
      hasMessage_ = true;
    }
  }
}

void ZigbeeManager::takeMessage(uint8_t* out, uint8_t maxLen, uint8_t& len) {
  if (!hasMessage_ || out == nullptr) {
    len = 0;
    return;
  }
  len = lastLen_ > maxLen ? maxLen : lastLen_;
  memcpy(out, lastMsg_, len);
  hasMessage_ = false;
}

}  // namespace sema
#endif  // SEMA_USE_ZIGBEE
