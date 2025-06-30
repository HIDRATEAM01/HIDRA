import { defineStore } from "pinia";
import { ref } from "vue";

import { useNotificationStore } from "@/store/notificationStore";
import {
  getServerConfig,
  postServerConfig,
  postServerStatus,
} from "@/services/serverService";

export const useServerStore = defineStore("server", () => {
  const ssid = ref("");
  const pass = ref("");
  const ip = ref("");
  const status = ref("");

  const loading = ref(false);
  const error = ref(null);
  const isEditMode = ref(false);

  const notificationStore = useNotificationStore();

  const backup = ref({});

  const fields = [
    { name: "ssid", label: "Rede", model: ssid },
    { name: "pass", label: "Senha", model: pass, type: "password" },
    { name: "ip", label: "IP", model: ip },
  ];

  // #region functions
  function holdBackup() {
    backup.value = {
      ssid: ssid.value,
      pass: pass.value,
      ip: ip.value,
    };
  }

  function restoreBackup() {
    ssid.value = backup.value.ssid;
    pass.value = backup.value.pass;
    ip.value = backup.value.ip;
  }
  // #endregion

  // #region requests
  async function saveServer() {
    loading.value = true;
    error.value = null;
    isEditMode.value = false;

    try {
      const response = await postServerConfig({
        ssid: ssid.value,
        pass: pass.value,
      });

      await fetch();

      notificationStore.raise(
        "Configurações de servidor salvas com sucesso!",
        "success",
        response.data
      );
    } catch (err) {
      error.value = "Erro ao salvar configurações de servidor.";
      notificationStore.raise(error.value, "error", err.message);
    } finally {
      loading.value = false;
    }
  }

  async function toggleServer() {
    loading.value = true;
    error.value = null;
    isEditMode.value = false;

    try {
      const novoStatus = status.value ? 0 : 1;
      const response = await postServerStatus({
        status: novoStatus,
      });

      await fetch();

      notificationStore.raise(
        novoStatus === 0
          ? "Servidor desativado com sucesso!"
          : "Servidor ativado com sucesso!",
        "success",
        response.data
      );
    } catch (err) {
      error.value = "Erro ao alterar configurações de servidor.";
      notificationStore.raise(error.value, "error", err.message);
    } finally {
      loading.value = false;
    }
  }

  async function fetch() {
    loading.value = true;
    error.value = null;
    isEditMode.value = false;

    const premise = getServerConfig();

    try {
      const response = await premise;

      if (response.status !== 200) {
        error.value = "Erro ao buscar configuração do servidor.";
        notificationStore.addError(error.value, "error");
        throw new Error("Sem conexão.");
      }

      const data = await response.data;
      ssid.value = data.ssid;
      pass.value = data.pass;
      ip.value = data.ip;
      status.value = data.status == 1;
    } catch (err) {
      error.value = "Erro ao carregar Configurações do Servidor.";
      notificationStore.raise(error.value, "error", err.message);
    } finally {
      loading.value = false;
    }
  }
  // #endregion

  // #region clicks
  const primaryClick = () => {
    if (isEditMode.value) {
      saveServer();
    } else {
      toggleServer();
    }
  };

  const secondaryClick = () => {
    if (isEditMode.value) {
      restoreBackup();
    } else {
      holdBackup();
    }
    isEditMode.value = !isEditMode.value;
  };
  // #endregion

  return {
    fields,
    status,
    loading,
    error,
    isEditMode,
    fetch,
    primaryClick,
    secondaryClick,
  };
});
