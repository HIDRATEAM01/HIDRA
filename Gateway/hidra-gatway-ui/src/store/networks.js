import { defineStore } from "pinia";

import { getWifiStatus, getServerConfig } from "@/services/networks";
import { ref } from "vue";

export const useNetworkStore = defineStore("networks", () => {
  const serverSSID = ref("");
  const serverPass = ref("");
  const serverIP = ref("");
  const serverStatus = ref("");

  const wifiSSID = ref("");
  const wifiRSSI = ref("");
  const wifiIP = ref("");
  const wifiStatus = ref("");

  const loadingServerConfig = ref(false);
  const errorServerConfig = ref(null);

  const loadingWifiStatus = ref(false);
  const errorWifiStatus = ref(null);

  const backupServerConfig = ref({});

  function holdServerConfig() {
    backupServerConfig.value = {
      ssid: serverSSID.value,
      pass: serverPass.value,
      ip: serverIP.value,
    };
  }

  function restoreServerConfig() {
    serverSSID.value = backupServerConfig.value.ssid;
    serverPass.value = backupServerConfig.value.pass;
    serverIP.value = backupServerConfig.value.ip;
  }

  async function fetchServerConfig() {
    loadingServerConfig.value = true;
    errorServerConfig.value = null;

    const premise = getServerConfig();

    try {
      const response = await premise;

      if (response.status !== 200) {
        throw new Error("Sem conexão.");
      }

      const data = await response.data;
      serverSSID.value = data.ssid;
      serverPass.value = data.pass;
      serverIP.value = data.ip;
      serverStatus.value = data.status == 1;
    } catch (err) {
      errorServerConfig.value = err.message;
    } finally {
      loadingServerConfig.value = false;
    }
  }

  async function fetchWifiStatus() {
    loadingWifiStatus.value = true;
    errorWifiStatus.value = null;

    const premise = getWifiStatus();
    try {
      const response = await premise;

      if (response.status !== 200) {
        throw new Error("Sem conexão.");
      }

      const data = await response.data;
      wifiSSID.value = data.ssid;
      wifiRSSI.value = data.rssi;
      wifiIP.value = data.ip;
      wifiStatus.value = data.status == 1;
    } catch (err) {
      errorWifiStatus.value = err.message;
    } finally {
      loadingWifiStatus.value = false;
    }
  }

  return {
    // Server Config
    serverSSID,
    serverPass,
    serverIP,
    serverStatus,
    loadingServerConfig,
    errorServerConfig,
    fetchServerConfig,
    holdServerConfig,
    restoreServerConfig,

    // Wifi Status
    wifiSSID,
    wifiRSSI,
    wifiIP,
    wifiStatus,
    loadingWifiStatus,
    errorWifiStatus,
    fetchWifiStatus,
  };
});
