#ifndef STORE_MANAGER_H
#define STORE_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>

#include <vector>

struct WifiCredential {
  String ssid;
  String password;
};

class StoreManager {
  public:
  StoreManager();
  void begin(const char* ns);

  void addSavedNetwork(const String& ssid, const String& password);
  std::vector<WifiCredential> getSavedNetworks();
  void removeSavedNetwork(int index);
  void clearAllNetworks();
  int getNetworkCount();

  private:
  Preferences prefs;
};

extern StoreManager wifiStore;

#endif
