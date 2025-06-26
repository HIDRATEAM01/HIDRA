<template>
  <v-container class="bg-background" fluid>
    <v-row>
      <v-col cols="2">
        <img
          :src="require('@/assets/globo.png')"
          alt="globo"
          style="height: 100px; border-radius: 50%"
        />
      </v-col>
      <v-col cols="8" class="text-center">
        <h1 class="shadow-2 text-textmain">Monitoramento de Águas em Recife</h1>
        <h3 class="shadow-2 text-textmain">Configuração - Gateway</h3>
      </v-col>
    </v-row>
    <v-row class="px-4">
      <v-col>
        <ConfigBlock
          title="Servidor"
          icon="mdi-lan"
          :fields="[
            { name: 'ssid', label: 'Rede' },
            { name: 'pass', label: 'Senha' },
            { name: 'ip', label: 'IP' },
          ]"
          primaryButton="Ativar"
          secondaryButton="Editar"
        />
      </v-col>
      <v-col>
        <ConfigBlock
          title="Wi-Fi"
          icon="mdi-wifi"
          :fields="[
            { name: 'ssid', label: 'Rede' },
            { name: 'status', label: 'Status' },
            { name: 'ip', label: 'IP' },
          ]"
          primaryButton="Ativar"
          secondaryButton="Redes"
        />
      </v-col>
      <v-col>
        <ConfigBlock
          title="Configurações"
          icon="mdi-cog"
          :fields="[
            { name: 'date', label: 'Data', type: 'date' },
            { name: 'clock', label: 'Hora', type: 'time' },
            { name: 'address', label: 'Endereço' },
          ]"
          secondaryButton="Detalhes"
        />
      </v-col>
    </v-row>
    <v-row>
      <v-col> módulos aqui </v-col>
    </v-row>
  </v-container>
</template>

<script>
// @ is an alias to /src
import ConfigBlock from "@/components/ConfigBlock.vue";

export default {
  name: "HomeView",
  components: { ConfigBlock },
};
</script>
