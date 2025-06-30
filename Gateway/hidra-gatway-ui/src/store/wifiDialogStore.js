import { defineStore } from "pinia";
import { ref } from "vue";

import { useNotificationStore } from "@/store/notificationStore";
import { getWifiNetworks, postWifiConnect } from "@/services/wifiService";

export const useWifiDialogStore = defineStore("wifiDialog", () => {
  const notificationStore = useNotificationStore();
  const showDialog = ref(false);
  const loading = ref(false);
  const loadingNew = ref(false);
  const error = ref(null);

  const networks = ref([]);
  const connected = ref(null);

  function getNetwork(id) {
    return networks.value.find((network) => network.id === id);
  }

  function setConnected(data) {
    connected.value = {
      ssid: data.ssid,
      rssi: parseFloat(data.rssi.replace("db", "")),
      ip: data.ip,
      status: data.status,
      isConnected: true,
    };

    networks.value = networks.value.filter((n) => n.ssid !== data.ssid);
  }

  function fillWifiNetworks(nearData, savedData) {
    const nearMap = new Map();
    nearData.forEach((net) => {
      nearMap.set(net.ssid, { ssid: net.ssid, rssi: Number(net.rssi) });
    });

    const allSsids = new Set([
      ...nearData.map((n) => n.ssid),
      ...savedData.map((s) => s.ssid),
    ]);

    networks.value = Array.from(allSsids)
      .map((ssid, index) => {
        const near = nearMap.get(ssid);
        const saved = savedData.find((s) => s.ssid === ssid);

        return {
          id: index,
          ssid,
          rssi: near ? near.rssi : -Infinity,
          isSaved: !!saved,
          pass: saved?.pass || null,
        };
      })
      .sort((a, b) => b.rssi - a.rssi);
  }

  // #region request
  async function tryConnect(ssid, pass) {
    console.log("Trying to connect to", ssid, "with password", pass);
    loadingNew.value = true;
    error.value = null;

    try {
      const response = await postWifiConnect({
        ssid: ssid,
        pass: pass,
      });

      await fetch();

      notificationStore.raise(
        "Conexão realizada com sucesso!",
        "success",
        response.data
      );
    } catch (err) {
      error.value = "Erro ao tentar se conectar a rede.";
      notificationStore.raise(error.value, "error", err.message);
    } finally {
      loadingNew.value = false;
    }
  }

  async function fetch() {
    loading.value = true;
    error.value = null;

    const premise = getWifiNetworks();

    try {
      const response = await premise;

      if (response.status !== 200) {
        error.value = "Erro ao buscar configuração de Wifi.";
        notificationStore.addError(error.value, "error");
        throw new Error("Sem conexão.");
      }

      const data = await response.data;
      fillWifiNetworks(data.near, data.saved);
    } catch (err) {
      error.value = "Erro ao carregar Configurações do Wifi.";
      notificationStore.raise(error.value, "error", err.message);
    } finally {
      loading.value = false;
    }
  }
  // #endregion

  return {
    loading,
    loadingNew,
    error,
    showDialog,
    networks,
    connected,
    fetch,
    getNetwork,
    setConnected,
    tryConnect,
  };
});
