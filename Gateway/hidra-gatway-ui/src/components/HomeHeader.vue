<template>
  <v-container fluid class="pa-0" style="max-width: 1000px; min-width: 350px">
    <v-row class="text-center" align="center" justify="center">
      <v-col cols="12" md="2" class="d-flex justify-center justify-md-start">
        <img
          :src="require('@/assets/hidra-logo.png')"
          alt="HIDRA"
          style="height: 100px; border-radius: 50%"
        />
      </v-col>
      <v-col cols="12" md="8">
        <h1 class="shadow-2 text-textmain">Monitoramento de Águas em Recife</h1>
        <h3 class="shadow-2 text-textmain">Configuração - Gateway</h3>
      </v-col>
      <v-col cols="2" />
    </v-row>
  </v-container>
</template>

<script>
export default {
  name: "HomeHeader",
};
</script>
