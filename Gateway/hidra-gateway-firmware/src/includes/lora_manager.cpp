#include "lora_manager.h"

LoraManager::LoraManager() {}

void LoraManager::begin(long frequency, int power) {
  LoRa.setPins(5, 14, 2);  //  (SS, RST, DIO0)
  LoRa.begin(frequency);
  LoRa.setTxPower(power);
}

bool LoraManager::sendMessage(uint32_t destination, const std::vector<uint8_t>& data) {
  std::vector<uint8_t> buffer = data;
  if (!encrypt(buffer)) return false;

  LoRa.beginPacket();
  LoRa.write((uint8_t*)&destination, sizeof(destination));
  LoRa.write(buffer.data(), buffer.size());
  LoRa.endPacket();

  return true;
}

bool LoraManager::receiveMessage(uint32_t& sender, std::vector<uint8_t>& data) {
  int packetSize = LoRa.parsePacket();
  if (packetSize < 4) return false;

  sender = 0;
  for (int i = 0; i < 4; ++i) {
    sender |= (LoRa.read() << (8 * i));
  }

  data.clear();
  while (LoRa.available()) {
    data.push_back(LoRa.read());
  }

  return decrypt(data);
}

bool LoraManager::receiveAsEmitter(uint32_t& sender, std::vector<uint8_t>& data) {
  return receiveMessage(sender, data);
}

bool LoraManager::receiveAsReceiver(uint32_t& sender, std::vector<uint8_t>& data) {
  if (!receiveMessage(sender, data)) return false;

  if (!deviceExists(sender)) {
    serial.log(LOG_ERROR, "[LoraManager] Dispositivo não autorizado: ", String(sender, HEX).c_str());
    return false;
  }

  return true;
}

bool LoraManager::addDevice(uint32_t address, const std::string& name) {
  if (deviceExists(address)) return false;
  devices.push_back({address, name});
  return true;
}

bool LoraManager::removeDevice(uint32_t address) {
  for (auto it = devices.begin(); it != devices.end(); ++it) {
    if (it->address == address) {
      devices.erase(it);
      return true;
    }
  }
  return false;
}

bool LoraManager::deviceExists(uint32_t address) const {
  for (const auto& d : devices) {
    if (d.address == address) return true;
  }
  return false;
}

bool LoraManager::loadEncryptionKey(const String& filePath) {
  JsonDocument doc;
  if (!fm.getEncryptionFile(doc, filePath.c_str())) return false;

  const char* keyHex = doc["aes_key"];
  const char* ivHex = doc["iv"];

  if (!keyHex || strlen(keyHex) != 32) return false;

  encryptionKey.clear();
  for (int i = 0; i < 16; ++i) {
    String byteStr = String(keyHex[i * 2]) + keyHex[i * 2 + 1];
    encryptionKey.push_back(strtol(byteStr.c_str(), nullptr, 16));
  }

  if (ivHex && strlen(ivHex) == 32) {
    for (int i = 0; i < 16; ++i) {
      String byteStr = String(ivHex[i * 2]) + ivHex[i * 2 + 1];
      encryptionKey.push_back(strtol(byteStr.c_str(), nullptr, 16));
    }
  }

  return true;
}

void LoraManager::setEncryptionKey(const std::vector<uint8_t>& key) {
  encryptionKey = key;
}

std::vector<uint8_t> LoraManager::getEncryptionKey() const {
  return encryptionKey;
}

bool LoraManager::encrypt(std::vector<uint8_t>& data) const {
  if (encryptionKey.size() != 16) return false;

  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_enc(&aes, encryptionKey.data(), 128);

  std::vector<uint8_t> iv(16, 0x00);
  if (encryptionKey.size() == 32) {
    std::copy(encryptionKey.begin() + 16, encryptionKey.end(), iv.begin());
  }

  uint8_t stream_block[16];
  size_t nc_off = 0;

  data.resize((data.size() + 15) & ~15);  // padding zero até múltiplo de 16
  std::vector<uint8_t> out(data.size());

  mbedtls_aes_crypt_ctr(&aes, data.size(), &nc_off, iv.data(), stream_block, data.data(), out.data());

  mbedtls_aes_free(&aes);
  data = out;
  return true;
}

bool LoraManager::decrypt(std::vector<uint8_t>& data) const {
  return encrypt(data);  // Aparentemente CTR é simétrico
}
