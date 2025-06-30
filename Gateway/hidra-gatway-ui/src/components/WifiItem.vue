<template>
  <v-list-item
    class="bg-secondary"
    :class="{
      'border-start border-primary': isConnected,
      'opacity-75': network.rssi < -90,
    }"
    :ripple="false"
    @click.prevent
  >
    <template #prepend>
      <v-tooltip location="top" content-class="mini-tooltip">
        <template #activator="{ props }">
          <v-icon v-bind="props" :icon="wifiIcon" />
        </template>
        {{ wifiTooltip }}
      </v-tooltip>
    </template>

    <template #default>
      <div>
        <v-row justify="center" class="my-1"> {{ network.ssid }} </v-row>
        <v-row justify="center" class="text-caption text-grey my-0">
          <span v-if="isConnected">Conectado</span>
          <span v-else-if="network.isSaved">Salvo</span>
        </v-row>

        <v-expand-transition>
          <v-btn
            v-if="showField && network.isSaved"
            variant="elevated"
            color="primary"
            size="small"
            @click="tryConnect"
          >
            Conectar
          </v-btn>
          <v-text-field
            v-else-if="showField"
            v-model="password"
            label="Senha"
            variant="underlined"
            density="comfortable"
            hide-details
            :type="showPass ? 'text' : 'password'"
          >
            <template v-slot:append-inner>
              <v-icon
                :icon="showPass ? 'mdi-eye' : 'mdi-eye-off'"
                @click="showPass = !showPass"
                size="small"
              />
            </template>
          </v-text-field>
        </v-expand-transition>
      </div>
    </template>

    <template #append>
      <div class="d-flex flex-column align-center ml-2">
        <WifiItemButton
          v-if="isConnected"
          type="disconnect"
          @click="disconnect"
        />
        <WifiItemButton
          v-else-if="network.isSaved"
          type="options"
          childType="delete"
          :show-child="showField"
          @click="showField = !showField"
          @childClick="removeSaved"
        />
        <WifiItemButton
          v-else
          type="options"
          childType="connect"
          :show-child="showField"
          @click="showField = !showField"
          @childClick="tryConnect"
        />
      </div>
    </template>
  </v-list-item>
</template>

<script>
import { computed, ref, watch } from "vue";
import { useWifiDialogStore } from "@/store/wifiDialogStore";
import WifiItemButton from "@/components/WifiItemButton.vue";

export default {
  name: "WifiItem",
  props: {
    network: Object,
    isConnected: Boolean,
  },
  components: {
    WifiItemButton,
  },
  setup(props) {
    const wifiDialogStore = useWifiDialogStore();
    const showField = ref(false);
    const showPass = ref(false);

    const password = ref("");

    watch(
      () => showField.value,
      (newValue) => {
        if (!newValue) {
          password.value = "";
          showPass.value = false;
        }
      }
    );

    const wifiTooltip = computed(() => {
      if (!props.network || props.network.rssi < -100) return "Fora do alcance";
      return `${props.network.rssi} dBm`;
    });

    const wifiIcon = computed(() => {
      if (!props.network || props.network.rssi < -100) return "mdi-wifi-remove";
      if (props.network.rssi < -80) return "mdi-wifi-strength-outline";
      if (props.network.rssi < -70) return "mdi-wifi-strength-1";
      if (props.network.rssi < -60) return "mdi-wifi-strength-2";
      if (props.network.rssi < -50) return "mdi-wifi-strength-3";
      if (props.network.rssi >= -50) return "mdi-wifi-strength-4";
      return "mdi-wifi";
    });

    const tryConnect = () => {
      if (props.network.isSaved) {
        wifiDialogStore.tryConnect(props.network.ssid, password.value);
      } else {
        wifiDialogStore.tryConnect(props.network.ssid, password.value);
      }
    };

    const disconnect = () => {
      //wifiDialogStore.disconnect(props.network.ssid);
      console.log(`Desconectando de ${props.network.ssid}`);
      showField.value = false;
    };

    const removeSaved = () => {
      //wifiDialogStore.removeSaved(props.network.ssid);
      console.log(`Removendo rede salva: ${props.network.ssid}`);
      showField.value = false;
    };

    return {
      wifiDialogStore,
      showField,
      showPass,
      wifiIcon,
      wifiTooltip,
      password,
      tryConnect,
      disconnect,
      removeSaved,
    };
  },
};
</script>
