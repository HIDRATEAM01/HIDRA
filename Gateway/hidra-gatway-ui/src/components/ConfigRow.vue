<template>
  <v-container fluid class="pb-4">
    <v-row class="px-4" justify="center">
      <v-col cols="auto">
        <ConfigBlock
          title="Servidor"
          icon="mdi-lan"
          :fields="serverFields"
          :formModel="serverFormModel"
          :readonly="serverReadOnly"
          :primaryButton="serverReadOnly ? 'Ativar' : 'Salvar'"
          :secondaryButton="serverReadOnly ? 'Editar' : 'Cancelar'"
          @secondaryClick="toggleServerEdit"
        />
      </v-col>
      <v-col cols="auto">
        <ConfigBlock
          title="Wi-Fi"
          icon="mdi-wifi"
          :fields="wifiFields"
          :formModel="wifiFormModel"
          :readonly="wifiReadOnly"
          primaryButton="Ativar"
          secondaryButton="Redes"
        />
      </v-col>
      <v-col cols="auto">
        <ConfigBlock
          title="Configurações"
          icon="mdi-cog"
          :fields="configFields"
          :formModel="configFormModel"
          :readonly="configReadOnly"
          :primaryButton="configReadOnly ? 'Detalhes' : 'Salvar'"
          :secondaryButton="configReadOnly ? 'Editar' : 'Cancelar'"
          @secondaryClick="toggleConfigEdit()"
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
    const configReadOnly = ref(true);
    const serverReadOnly = ref(true);
    const wifiReadOnly = ref(true);

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
      loading: networkData.loadingWifiStatus,
      error: networkData.errorWifiStatus,
    };

    const serverFormModel = {
      ssid: networkData.serverSSID,
      pass: networkData.serverPass,
      ip: networkData.serverIP,
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

    const toggleConfigEdit = () => {
      configReadOnly.value = !configReadOnly.value;
    };

    const toggleServerEdit = () => {
      serverReadOnly.value = !serverReadOnly.value;
    };

    return {
      // Fields for each configuration block
      serverFields,
      wifiFields,
      configFields,

      // Server block relative data
      serverFormModel,
      serverReadOnly,
      toggleServerEdit,

      // Wifi block relative data
      wifiFormModel,
      wifiReadOnly,

      // Config block relative data
      configFormModel,
      configReadOnly,
      toggleConfigEdit,
    };
  },
};
</script>
