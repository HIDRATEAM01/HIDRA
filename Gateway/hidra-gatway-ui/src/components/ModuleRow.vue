<template>
  <v-container fluid class="pb-4">
    <v-row v-if="moduleStore.loading" justify="center" class="mt-4">
      <v-col cols="auto">
        <v-progress-circular indeterminate color="primary" size="64" />
        <div class="mt-4">Carregando...</div>
      </v-col>
    </v-row>
    <v-row v-else class="px-4" justify="center">
      <v-col cols="auto" v-for="module in moduleStore.modules" :key="module.id">
        <ModuleBlock :title="module.name" :values="module" />
      </v-col>
    </v-row>
  </v-container>
</template>

<script>
import ModuleBlock from "@/components/ModuleBlock.vue";
import { useModuleStore } from "@/store/moduleStore";
import { onMounted } from "vue";
export default {
  name: "ModuleRow",
  components: { ModuleBlock },
  setup() {
    const moduleStore = useModuleStore();

    onMounted(() => {
      moduleStore.fetch();
    });

    return {
      moduleStore,
    };
  },
};
</script>
