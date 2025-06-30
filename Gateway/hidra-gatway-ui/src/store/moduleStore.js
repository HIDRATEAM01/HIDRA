import { defineStore } from "pinia";
import { ref } from "vue";

import { getModules } from "@/services/moduleService";
import { useNotificationStore } from "@/store/notificationStore";

export const useModuleStore = defineStore("module", () => {
  const loading = ref(false);
  const error = ref(null);
  const modules = ref([]);

  const notificationStore = useNotificationStore();

  function mountModules(modulesData) {
    return modulesData.map((module) => ({
      id: module.id,
      name: module.name,
      recieveTime: `${module["recieve-date"]} ${module["recieve-time"]}`,
      address: module.address,
      bat: module.bat,
    }));
  }

  //#region requests
  async function fetch() {
    loading.value = true;
    error.value = null;

    const premise = getModules();
    try {
      const response = await premise;

      if (response.status !== 200) {
        error.value = "Erro ao carregar configurações.";
        notificationStore.raise(error.value, "error");
        throw new Error("Sem conexão.");
      }
      modules.value = mountModules(response.data.modules);
    } catch (err) {
      error.value = "Erro ao carregar configurações.";
      notificationStore.raise(error.value, "error", err.message);
    } finally {
      loading.value = false;
    }
  }
  //#endregion

  function openModule(idModule) {
    const module = modules.value.find((mod) => mod.id === idModule);
    if (module) {
      // TODO: Implement module opening logic
      notificationStore.raise(`Abrindo módulo: ${module.name}`, "info");
    }
  }

  return {
    loading,
    error,
    modules,
    fetch,
    openModule,
  };
});
