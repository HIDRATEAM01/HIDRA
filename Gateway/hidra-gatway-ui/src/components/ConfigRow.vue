<template>
  <v-container fluid class="pb-4">
    <v-row class="px-4" justify="center">
      <v-col cols="auto">
        <ConfigBlock
          title="Servidor"
          icon="mdi-lan"
          :fields="serverStore.fields"
          :loading="serverStore.loading"
          :readonly="!serverStore.isEditMode"
          :primaryButton="serverPrimaryButton"
          :secondaryButton="serverStore.isEditMode ? 'Cancelar' : 'Editar'"
          @primaryClick="serverStore.primaryClick"
          @secondaryClick="serverStore.secondaryClick"
        />
      </v-col>
      <v-col cols="auto">
        <ConfigBlock
          title="Wi-Fi"
          icon="mdi-wifi"
          :fields="wifiStore.fields"
          :loading="wifiStore.loading"
          :readonly="true"
          :primaryButton="wifiStore.status ? 'Desativar' : 'Ativar'"
          secondaryButton="Redes"
          @primaryClick="wifiStore.primaryClick"
          @secondaryClick="wifiStore.secondaryClick"
        />
      </v-col>
      <v-col cols="auto">
        <ConfigBlock
          title="Configurações"
          icon="mdi-cog"
          :fields="configStore.fields"
          :loading="configStore.loading"
          :readonly="!configStore.isEditMode"
          :primaryButton="configStore.isEditMode ? 'Salvar' : 'Detalhes'"
          :secondaryButton="configStore.isEditMode ? 'Cancelar' : 'Editar'"
          @primaryClick="configStore.primaryClick"
          @secondaryClick="configStore.secondaryClick"
        />
      </v-col>
    </v-row>
  </v-container>
</template>

<script>
import ConfigBlock from "@/components/ConfigBlock.vue";
import { useConfigStore } from "@/store/configStore";
import { useServerStore } from "@/store/serverStore";
import { useWifiStore } from "@/store/wifiStore";
import { computed, onMounted } from "vue";

export default {
  name: "ConfigRow",
  components: { ConfigBlock },
  setup() {
    const configStore = useConfigStore();
    const serverStore = useServerStore();
    const wifiStore = useWifiStore();

    onMounted(async () => {
      await Promise.all([
        wifiStore.fetch(),
        serverStore.fetch(),
        configStore.fetch(),
      ]);
    });

    const serverPrimaryButton = computed(() => {
      if (serverStore.isEditMode) {
        return "Salvar";
      }
      return serverStore.status ? "Desativar" : "Ativar";
    });

    return {
      configStore,
      serverStore,
      wifiStore,
      serverPrimaryButton,
    };
  },
};
</script>
