#ifndef LORA_MANAGER_H
#define LORA_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LoRa.h>
#include <SPI.h>
#include <mbedtls/aes.h>

#include <vector>

#include "file_manager.h"
#include "serial_manager.h"

struct DeviceInfo {
  uint32_t address;
  std::string name;
};

class LoraManager {
  public:
  LoraManager();

  void begin(long frequency, int power = 14);
  bool sendMessage(uint32_t destination, const std::vector<uint8_t>& data);
  bool receiveMessage(uint32_t& sender, std::vector<uint8_t>& data);

  bool receiveAsReceiver(uint32_t& sender, std::vector<uint8_t>& data);
  bool receiveAsEmitter(uint32_t& sender, std::vector<uint8_t>& data);

  bool addDevice(uint32_t address, const std::string& name);
  bool removeDevice(uint32_t address);
  bool deviceExists(uint32_t address) const;

  bool loadEncryptionKey(const String& filePath);
  bool saveEncryptionKey(const String& filePath);
  void setEncryptionKey(const std::vector<uint8_t>& key);
  std::vector<uint8_t> getEncryptionKey() const;

  private:
  std::vector<DeviceInfo> devices;
  std::vector<uint8_t> encryptionKey;

  bool encrypt(std::vector<uint8_t>& data) const;
  bool decrypt(std::vector<uint8_t>& data) const;
};

#endif