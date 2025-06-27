<template>
  <v-container class="bg-background" fluid style="min-height: 100vh">
    <HomeHeader />
    <ConfigRow />
    <v-row>
      <v-col cols="12" class="text-center mt-4">
        <h2 class="shadow-2 text-textmain">Últimas Conexões</h2>
      </v-col>
    </v-row>
    <ModuleRow />
  </v-container>
</template>

<script>
// @ is an alias to /src
import HomeHeader from "@/components/HomeHeader.vue";
import ConfigRow from "@/components/ConfigRow.vue";
import ModuleRow from "@/components/ModuleRow.vue";

export default {
  name: "HomeView",
  components: { ModuleRow, HomeHeader, ConfigRow },
};
</script>
