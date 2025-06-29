import { defineStore } from "pinia";

import { useNotificationStore } from "@/store/notificationStore";
import { useWifiDialogStore } from "./wifiDialogStore";
import { getWifiStatus, postWifiStatus } from "@/services/wifiService";
import { ref } from "vue";

export const useWifiStore = defineStore("wifi", () => {
  const ssid = ref("");
  const rssi = ref("");
  const ip = ref("");
  const status = ref(false);

  const loading = ref(false);
  const error = ref(null);

  const notificationStore = useNotificationStore();
  const wifiDialogStore = useWifiDialogStore();

  // Fields in home page
  const fields = [
    { name: "ssid", label: "Rede", model: ssid },
    { name: "rssi", label: "Sinal", model: rssi },
    { name: "ip", label: "IP", model: ip },
  ];

  function fillData(data) {
    ssid.value = data.ssid || "Desconectado";
    rssi.value = data.rssi ? `${data.rssi} db` : "Desconectado";
    ip.value = data.ip || "Desconectado";
    status.value = data.status === 1;

    wifiDialogStore.setConnected({
      ssid: "wifi-teste",
      rssi: "-30db",
      ip: "192.168.4.1",
      status: true,
    });
  }

  // #region requests
  async function toggleWifi() {
    loading.value = true;
    error.value = null;

    try {
      const novoStatus = status.value ? 0 : 1;
      const response = await postWifiStatus({
        status: novoStatus,
      });

      await fetch();

      notificationStore.raise(
        novoStatus === 0
          ? "Wifi desativado com sucesso!"
          : "Wifi ativado com sucesso!",
        "success",
        response.data
      );
    } catch (err) {
      error.value = "Erro ao alterar configurações de Wifi.";
      notificationStore.raise(error.value, "error", err.message);
    } finally {
      loading.value = false;
    }
  }

  async function fetch() {
    loading.value = true;
    error.value = null;

    const premise = getWifiStatus();

    try {
      const response = await premise;

      if (response.status !== 200) {
        error.value = "Erro ao buscar configuração de Wifi.";
        notificationStore.addError(error.value, "error");
        throw new Error("Sem conexão.");
      }

      const data = await response.data;
      fillData(data);
    } catch (err) {
      error.value = "Erro ao carregar Configurações do Wifi.";
      notificationStore.raise(error.value, "error", err.message);
    } finally {
      loading.value = false;
    }
  }
  // #endregion

  // #region clicks
  const primaryClick = () => {
    toggleWifi();
  };

  const secondaryClick = () => {
    wifiDialogStore.showDialog = true;
  };
  // #endregion

  return {
    fields,
    status,
    loading,
    error,
    fetch,
    primaryClick,
    secondaryClick,
  };
});
