<template>
  <v-container class="bg-background" fluid style="min-height: 100vh">
    <HomeHeader />
    <ConfigRow />
    <v-row>
      <v-col cols="12" class="text-center mt-6">
        <h2 class="shadow-2 text-textmain">Últimas Conexões</h2>
      </v-col>
    </v-row>
    <ModuleRow />
    <WifiDialog />
  </v-container>
</template>

<script>
// @ is an alias to /src
import HomeHeader from "@/components/HomeHeader.vue";
import ConfigRow from "@/components/ConfigRow.vue";
import ModuleRow from "@/components/ModuleRow.vue";
import WifiDialog from "@/components/WifiDialog.vue";

export default {
  name: "HomeView",
  components: { ModuleRow, HomeHeader, ConfigRow, WifiDialog },
};
</script>
