import { defineStore } from "pinia";
import { computed, ref } from "vue";

import { getConfig, postConfigTime } from "@/services/configService";
import { updateClock, getDateString, getTimeString } from "@/utils/date";
import { useNotificationStore } from "@/store/notificationStore";

export const useConfigStore = defineStore("config", () => {
  let timer = null;

  const clock = ref(null);
  const address = ref("");
  const loading = ref(false);
  const error = ref(null);
  const isEditMode = ref(false);

  const backup = ref({});
  const notificationStore = useNotificationStore();

  // #region Computed
  const date = computed({
    get() {
      return getDateString(clock.value);
    },
    set(value) {
      clock.value = updateClock(clock.value, value, null);
      console.log("date set", clock.value);
    },
  });

  const time = computed({
    get() {
      return getTimeString(clock.value);
    },
    set(value) {
      clock.value = updateClock(clock.value, null, value);
      console.log("time set", clock.value);
    },
  });
  // #endregion

  const fields = [
    { name: "date", label: "Data", model: date, type: "date" },
    { name: "time", label: "Hora", model: time, type: "time" },
    { name: "address", label: "Endereço", model: address },
  ];

  // #region functions
  function holdBackup() {
    backup.value = {
      clock: clock.value,
      address: address.value,
    };
  }

  function restoreBackup() {
    clock.value = backup.value.clock;
    address.value = backup.value.address;
  }

  function startClock() {
    if (timer) {
      clearInterval(timer);
    }
    if (clock.value) {
      timer = setInterval(() => {
        if (isEditMode.value) {
          backup.value.clock = new Date(backup.value.clock.getTime() + 1000);
        } else {
          clock.value = new Date(clock.value.getTime() + 1000);
        }
      }, 1000);
    }
  }
  // #endregion

  // #region requests
  async function saveConfig() {
    loading.value = true;
    error.value = null;
    isEditMode.value = false;

    try {
      const response = await postConfigTime({
        date: getDateString(clock.value),
        time: getTimeString(clock.value),
        address: address.value,
      });

      await fetch();

      notificationStore.raise(
        "Configurações salvas com sucesso!",
        "success",
        response.data
      );
    } catch (err) {
      error.value = "Erro ao salvar configurações.";
      notificationStore.raise(error.value, "error", err.message);
    } finally {
      loading.value = false;
    }
  }

  async function fetch() {
    loading.value = true;
    error.value = null;

    const premise = getConfig();
    try {
      const response = await premise;

      if (response.status !== 200) {
        error.value = "Erro ao carregar configurações.";
        notificationStore.raise(error.value, "error");
        throw new Error("Sem conexão.");
      }
      const data = await response.data;
      clock.value = updateClock(null, data.date, data.time);
      address.value = data.address;
      startClock();
    } catch (err) {
      error.value = "Erro ao carregar configurações.";
      notificationStore.raise(error.value, "error", err.message);
    } finally {
      loading.value = false;
    }
  }
  // #endregion

  // #region clicks
  const primaryClick = () => {
    if (isEditMode.value) {
      saveConfig();
    } else {
      //TODO: openDetails.value = true;
      notificationStore.raise("Funcionalidade ainda não implementada.", "info");
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
    loading,
    error,
    isEditMode,
    fetch,
    primaryClick,
    secondaryClick,
  };
});
