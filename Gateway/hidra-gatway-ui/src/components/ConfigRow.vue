<template>
  <v-container fluid class="pb-4">
    <v-row class="px-4" justify="center">
      <v-col cols="auto">
        <ConfigBlock
          title="Servidor"
          icon="mdi-lan"
          :fields="serverFields"
          primaryButton="Ativar"
          secondaryButton="Editar"
        />
      </v-col>
      <v-col cols="auto">
        <ConfigBlock
          title="Wi-Fi"
          icon="mdi-wifi"
          :fields="wifiFields"
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
import { storeToRefs } from "pinia";
import { onMounted, ref } from "vue";

export default {
  name: "ConfigRow",
  components: { ConfigBlock },
  setup() {
    const configReadOnly = ref(true);

    const serverFields = [
      { name: "ssid", label: "Rede" },
      { name: "pass", label: "Senha" },
      { name: "ip", label: "IP" },
    ];

    const wifiFields = [
      { name: "ssid", label: "Rede" },
      { name: "status", label: "Status" },
      { name: "ip", label: "IP" },
    ];

    const configFields = [
      { name: "date", label: "Data", type: "date" },
      { name: "time", label: "Hora", type: "time" },
      { name: "address", label: "Endereço" },
    ];

    const configStore = useConfigStore();

    onMounted(() => {
      configStore.fetchConfig();
    });

    const configData = storeToRefs(configStore);

    const toggleConfigEdit = () => {
      configReadOnly.value = !configReadOnly.value;
    };

    return {
      // Fields for each configuration block
      serverFields,
      wifiFields,
      configFields,

      // Config store rlative functions
      configData,
      configReadOnly,
      toggleConfigEdit,
    };
  },
};
</script>
