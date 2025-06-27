import { defineStore } from "pinia";
import { computed, ref } from "vue";

import { getConfig } from "@/services/config";
import { parseToDateTime } from "@/utils/date";

export const useConfigStore = defineStore("config", () => {
  const clock = ref(null);
  const address = ref("");
  const loading = ref(false);
  const error = ref(null);

  let timer = null;

  function startClock() {
    if (timer) {
      clearInterval(timer);
    }
    if (clock.value) {
      timer = setInterval(() => {
        clock.value = new Date(clock.value.getTime() + 1000);
      }, 1000);
    }
  }

  async function fetchConfig() {
    loading.value = true;
    error.value = null;
    try {
      const response = await getConfig();

      if (response.status !== 200) {
        throw new Error("Sem conexão.");
      }
      const data = await response.data;
      clock.value = parseToDateTime(data.date, data.time);
      address.value = data.address;
      startClock();
    } catch (err) {
      error.value = err.message;
    } finally {
      loading.value = false;
    }
  }

  const date = computed(() => {
    if (!clock.value) return "";
    const year = clock.value.getFullYear();
    const month = String(clock.value.getMonth() + 1).padStart(2, "0");
    const day = String(clock.value.getDate()).padStart(2, "0");
    return `${year}-${month}-${day}`;
  });

  const time = computed(() => {
    if (!clock.value) return null;
    return clock.value.toLocaleTimeString("pt-BR", {
      hour: "2-digit",
      minute: "2-digit",
      second: "2-digit",
      hour12: false,
    });
  });

  return { date, time, address, loading, error, fetchConfig };
});
