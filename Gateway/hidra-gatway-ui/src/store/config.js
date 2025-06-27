import { defineStore } from "pinia";
import { computed, ref } from "vue";

import { getConfig, postConfigTime } from "@/services/config";
import { updateClock, getDateString, getTimeString } from "@/utils/date";
import { useNotificationStore } from "@/store/notifications";

export const useConfigStore = defineStore("config", () => {
  const clock = ref(null);
  const address = ref("");
  const loading = ref(false);
  const error = ref(null);

  let timer = null;
  let isBackupActive = false;

  const backupServerConfig = ref({});
  const notificationStore = useNotificationStore();

  function holdConfig() {
    isBackupActive = true;
    backupServerConfig.value = {
      clock: clock.value,
      address: address.value,
    };
  }

  function restoreConfig() {
    isBackupActive = false;
    clock.value = backupServerConfig.value.clock;
    address.value = backupServerConfig.value.address;
  }

  function startClock() {
    if (timer) {
      clearInterval(timer);
    }
    if (clock.value) {
      timer = setInterval(() => {
        if (isBackupActive) {
          backupServerConfig.value.clock = new Date(
            backupServerConfig.value.clock.getTime() + 1000
          );
        } else {
          clock.value = new Date(clock.value.getTime() + 1000);
        }
      }, 1000);
    }
  }

  // Server requests

  async function saveConfig() {
    loading.value = true;
    error.value = null;

    try {
      const response = await postConfigTime({
        date: getDateString(clock.value),
        time: getTimeString(clock.value),
        address: address.value,
      });

      await fetchConfig();

      notificationStore.raise(
        "Configurações salvas com sucesso!",
        "success",
        response.data
      );
    } catch (err) {
      error.value = err.message;
      notificationStore.raise(
        "Erro ao salvar configurações.",
        "error",
        err.message
      );
    } finally {
      loading.value = false;
    }
  }

  async function fetchConfig() {
    loading.value = true;
    error.value = null;

    const premise = getConfig();
    try {
      const response = await premise;

      if (response.status !== 200) {
        throw new Error("Sem conexão.");
      }
      const data = await response.data;
      clock.value = updateClock(null, data.date, data.time);
      address.value = data.address;
      startClock();
    } catch (err) {
      error.value = err.message;
      notificationStore.raise(
        "Erro ao carregar configurações.",
        "error",
        err.message
      );
    } finally {
      loading.value = false;
    }
  }

  // Computed properties

  const date = computed({
    get() {
      return getDateString(clock.value);
    },
    set(value) {
      updateClock(clock.value, value, null);
    },
  });

  const time = computed({
    get() {
      return getTimeString(clock.value);
    },
    set(value) {
      updateClock(clock.value, null, value);
    },
  });

  return {
    date,
    time,
    address,
    loading,
    error,
    fetchConfig,
    holdConfig,
    saveConfig,
    restoreConfig,
  };
});
