<template>
  <v-container fluid class="pb-4">
    <v-row class="px-4" justify="center">
      <v-col cols="auto">
        <ConfigBlock
          title="Servidor"
          icon="mdi-lan"
          :fields="serverFields"
          :formModel="serverFormModel"
          :readonly="!serverEditMode"
          :primaryButton="
            serverEditMode
              ? 'Salvar'
              : serverFormModel.status.value
              ? 'Desativar'
              : 'Ativar'
          "
          :secondaryButton="serverEditMode ? 'Cancelar' : 'Editar'"
          @primaryClick="primaryServerClick"
          @secondaryClick="toggleServerEdit"
        />
      </v-col>
      <v-col cols="auto">
        <ConfigBlock
          title="Wi-Fi"
          icon="mdi-wifi"
          :fields="wifiFields"
          :formModel="wifiFormModel"
          :primaryButton="wifiFormModel.status.value ? 'Desativar' : 'Ativar'"
          secondaryButton="Redes"
          readonly
        />
      </v-col>
      <v-col cols="auto">
        <ConfigBlock
          title="Configurações"
          icon="mdi-cog"
          :fields="configFields"
          :formModel="configFormModel"
          :readonly="!configEditMode"
          :primaryButton="configEditMode ? 'Salvar' : 'Detalhes'"
          :secondaryButton="configEditMode ? 'Cancelar' : 'Editar'"
          @primaryClick="primaryConfigClick"
          @secondaryClick="toggleConfigEdit"
        />
      </v-col>
    </v-row>
  </v-container>
</template>

<script>
import ConfigBlock from "@/components/ConfigBlock.vue";
import { useConfigStore } from "@/store/config";
import { useNetworkStore } from "@/store/networks";
import { storeToRefs } from "pinia";
import { onMounted, ref } from "vue";

export default {
  name: "ConfigRow",
  components: { ConfigBlock },
  setup() {
    const configEditMode = ref(false);
    const serverEditMode = ref(false);

    const serverFields = [
      { name: "ssid", label: "Rede" },
      { name: "pass", label: "Senha" },
      { name: "ip", label: "IP" },
    ];

    const wifiFields = [
      { name: "ssid", label: "Rede" },
      { name: "rssi", label: "Força" },
      { name: "ip", label: "IP" },
    ];

    const configFields = [
      { name: "date", label: "Data", type: "date" },
      { name: "time", label: "Hora", type: "time" },
      { name: "address", label: "Endereço" },
    ];

    const configStore = useConfigStore();
    const networkStore = useNetworkStore();
    const configData = storeToRefs(configStore);
    const networkData = storeToRefs(networkStore);

    const configFormModel = {
      date: configData.date,
      time: configData.time,
      address: configData.address,
      loading: configData.loading,
      error: configData.error,
    };

    const wifiFormModel = {
      ssid: networkData.wifiSSID,
      rssi: networkData.wifiRSSI,
      ip: networkData.wifiIP,
      status: networkData.wifiStatus,
      loading: networkData.loadingWifiStatus,
      error: networkData.errorWifiStatus,
    };

    const serverFormModel = {
      ssid: networkData.serverSSID,
      pass: networkData.serverPass,
      ip: networkData.serverIP,
      status: networkData.serverStatus,
      loading: networkData.loadingServerConfig,
      error: networkData.errorServerConfig,
    };

    onMounted(async () => {
      await Promise.all([
        networkStore.fetchServerConfig(),
        networkStore.fetchWifiStatus(),
        configStore.fetchConfig(),
      ]);
    });

    const primaryConfigClick = () => {
      if (configEditMode.value) {
        configStore.saveConfig();
        configEditMode.value = false;
      } else {
        // TODO: Open config details modal
        console.log("Open config details modal");
      }
    };

    const toggleConfigEdit = () => {
      if (configEditMode.value) {
        configStore.restoreConfig();
      } else {
        configStore.holdConfig();
      }
      configEditMode.value = !configEditMode.value;
    };

    const primaryServerClick = () => {
      if (serverEditMode.value) {
        networkStore.saveServerConfig();
      } else {
        //TODO: Toggle server status
        console.log("Toggle server status");
      }
    };

    const toggleServerEdit = () => {
      if (serverEditMode.value) {
        networkStore.restoreServerConfig();
      } else {
        networkStore.holdServerConfig();
      }

      serverEditMode.value = !serverEditMode.value;
    };

    return {
      // Fields for each configuration block
      serverFields,
      wifiFields,
      configFields,

      // Server block relative data
      serverFormModel,
      serverEditMode,
      toggleServerEdit,
      primaryServerClick,

      // Wifi block relative data
      wifiFormModel,

      // Config block relative data
      configFormModel,
      configEditMode,
      toggleConfigEdit,
      primaryConfigClick,
    };
  },
};
</script>
