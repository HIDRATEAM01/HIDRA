import { defineStore } from "pinia";
import { ref } from "vue";

export const useNotificationStore = defineStore("notification", () => {
  const queue = ref([]);

  function raise(msg, type = "error", response = "", silent = false) {
    if (!silent)
      queue.value.push({
        text: msg,
        color: type,
        timeout: 5000,
      });

    const responseText =
      typeof response === "object" ? JSON.stringify(response) : response;

    if (type == "error") console.error(`${msg} (${responseText})`);
    else if (type == "success") console.log(`${msg} (${responseText})`);
    else console.info(`${msg} (${responseText})`);
  }

  return { queue, raise };
});
