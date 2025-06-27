<template>
  <v-container fluid class="pb-4">
    <v-row class="px-4" justify="center">
      <v-col cols="auto">
        <ModuleBlock title="Módulo 1" />
      </v-col>
      <v-col cols="auto">
        <ModuleBlock title="Módulo 2" />
      </v-col>
      <v-col cols="auto">
        <ModuleBlock title="Módulo 3" />
      </v-col>
      <v-col cols="auto">
        <ModuleBlock title="Módulo 4" />
      </v-col>
      <v-col cols="auto">
        <ModuleBlock title="Módulo 5" />
      </v-col>
    </v-row>
  </v-container>
</template>

<script>
import ModuleBlock from "@/components/ModuleBlock.vue";
export default {
  name: "ModuleRow",
  components: { ModuleBlock },
};
</script>
