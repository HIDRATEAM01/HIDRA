<template>
  <v-container fluid class="pb-4">
    <v-row>
      <v-col cols="2">
        <img
          :src="require('@/assets/hidra-logo.png')"
          alt="HIDRA"
          style="height: 100px; border-radius: 50%"
        />
      </v-col>
      <v-col cols="8" class="text-center">
        <h1 class="shadow-2 text-textmain">Monitoramento de Águas em Recife</h1>
        <h3 class="shadow-2 text-textmain">Configuração - Gateway</h3>
      </v-col>
    </v-row>
  </v-container>
</template>

<script>
export default {
  name: "HomeHeader",
};
</script>
