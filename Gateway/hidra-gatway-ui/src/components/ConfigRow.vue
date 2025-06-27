<template>
  <v-container fluid class="pb-4">
    <v-row class="px-4" justify="center">
      <v-col cols="auto">
        <ConfigBlock
          title="Servidor"
          icon="mdi-lan"
          :fields="serverFields"
          :formModel="networkData"
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
          :formModel="networkData"
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
          :formModel="configData"
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
      { name: "serverSSID", label: "Rede" },
      { name: "serverPass", label: "Senha" },
      { name: "serverIP", label: "IP" },
    ];

    const wifiFields = [
      { name: "wifiSSID", label: "Rede" },
      { name: "wifiRSSI", label: "Força" },
      { name: "wifiIP", label: "IP" },
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

      // Config store rlative data
      configData,
      configReadOnly,
      toggleConfigEdit,

      // Network store relative data
      networkData,
      serverReadOnly,
      wifiReadOnly,
      toggleServerEdit,
    };
  },
};
</script>
