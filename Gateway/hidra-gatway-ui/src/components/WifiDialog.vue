<template>
  <v-dialog
    width="auto"
    v-model="wifiDialogStore.showDialog"
    scrim="black"
    persistent
  >
    <v-card class="pa-4">
      <v-btn
        icon="mdi-close"
        variant="text"
        class="position-absolute"
        style="top: 8px; right: 8px"
        @click="wifiDialogStore.showDialog = false"
      />

      <!-- Title -->
      <v-card-title class="ml-6 mr-12 align-center">
        <v-icon icon="mdi-wifi" class="mr-2 mb-2" />
        Configurações de Rede
      </v-card-title>

      <!-- Content -->
      <v-card-text style="max-height: 70vh; overflow-y: auto">
        <!-- Rede conectada -->
        <template v-if="showWifiNetwork">
          <h4 class="mb-2">Rede Conectada</h4>
          <WifiItem :network="wifiDialogStore.connected" is-connected />
          <v-divider class="my-4" />
        </template>
        <template v-if="showSkeletonNetwork">
          <h4 class="mb-2">Rede Conectada</h4>
          <v-skeleton-loader color="surface" type="list-item-avatar" />
          <v-divider class="my-4" />
        </template>

        <!-- Redes disponíveis -->
        <v-row class="justify-space-between align-center mb-2">
          <v-col>
            <h4 class="my-1 ml-8">Redes Disponíveis</h4>
          </v-col>
          <v-btn
            icon="mdi-refresh"
            size="x-small"
            variant="text"
            @click="wifiDialogStore.fetch"
          />
        </v-row>

        <!-- Lista de redes -->
        <v-row v-if="wifiDialogStore.loading" justify="center" class="mt-4">
          <v-col cols="auto">
            <v-progress-circular indeterminate color="primary" size="64" />
            <div class="mt-4">Carregando...</div>
          </v-col>
        </v-row>

        <v-list
          v-else
          nav
          max-height="300px"
          style="
            overflow-y: auto;
            scrollbar-width: thin;
            scrollbar-color: gray transparent;
          "
        >
          <WifiItem
            v-for="(net, i) in wifiDialogStore.networks"
            :key="i"
            :network="net"
          />
        </v-list>
      </v-card-text>
    </v-card>
  </v-dialog>
</template>

<script>
import { onMounted, computed } from "vue";
import { useWifiDialogStore } from "@/store/wifiDialogStore";

import WifiItem from "@/components/WifiItem.vue";

export default {
  name: "WifiDialog",
  components: {
    WifiItem,
  },
  setup: () => {
    const wifiDialogStore = useWifiDialogStore();

    onMounted(async () => {
      wifiDialogStore.fetch();
    });

    const showWifiNetwork = computed(() => {
      return wifiDialogStore.connected && !wifiDialogStore.loadingNew;
    });

    const showSkeletonNetwork = computed(() => {
      return wifiDialogStore.connected && wifiDialogStore.loadingNew;
    });

    return {
      wifiDialogStore,
      showWifiNetwork,
      showSkeletonNetwork,
    };
  },
};
</script>
