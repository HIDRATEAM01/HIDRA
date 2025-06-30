#include "store_manager.h"

StoreManager::StoreManager() {}

void StoreManager::begin(const char* ns) {
  prefs.begin(ns, false);
}

void StoreManager::addSavedNetwork(const String& ssid, const String& password) {
  int count = prefs.getInt("count", 0);
  prefs.putString(("ssid" + String(count)).c_str(), ssid);
  prefs.putString(("pass" + String(count)).c_str(), password);
  prefs.putInt("count", count + 1);
}

std::vector<WifiCredential> StoreManager::getSavedNetworks() {
  std::vector<WifiCredential> list;
  int count = prefs.getInt("count", 0);

  for (int i = 0; i < count; i++) {
    String ssid = prefs.getString(("ssid" + String(i)).c_str(), "");
    String pass = prefs.getString(("pass" + String(i)).c_str(), "");
    if (ssid.length() > 0) {
      list.push_back({ssid, pass});
    }
  }

  return list;
}

void StoreManager::removeSavedNetwork(int index) {
  int count = prefs.getInt("count", 0);
  if (index < 0 || index >= count) return;

  // move os itens seguintes para cima
  for (int i = index; i < count - 1; i++) {
    prefs.putString(("ssid" + String(i)).c_str(), prefs.getString(("ssid" + String(i + 1)).c_str(), ""));
    prefs.putString(("pass" + String(i)).c_str(), prefs.getString(("pass" + String(i + 1)).c_str(), ""));
  }

  // remove o último
  prefs.remove(("ssid" + String(count - 1)).c_str());
  prefs.remove(("pass" + String(count - 1)).c_str());

  prefs.putInt("count", count - 1);
}

void StoreManager::clearAllNetworks() {
  int count = prefs.getInt("count", 0);
  for (int i = 0; i < count; i++) {
    prefs.remove(("ssid" + String(i)).c_str());
    prefs.remove(("pass" + String(i)).c_str());
  }
  prefs.putInt("count", 0);
}

int StoreManager::getNetworkCount() {
  return prefs.getInt("count", 0);
}
