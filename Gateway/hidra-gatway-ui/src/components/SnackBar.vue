<template>
  <v-snackbar-queue
    v-model="queue"
    variant="tonal"
    location="top right"
    closable
    close-text="x"
  />
</template>

<script>
import { storeToRefs } from "pinia";
import { useNotificationStore } from "@/store/notifications";

export default {
  name: "AppSnackbar",
  setup() {
    const notificationStore = useNotificationStore();
    const { queue } = storeToRefs(notificationStore);

    return {
      queue,
    };
  },
};
</script>
