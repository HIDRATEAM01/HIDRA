<template>
  <v-tooltip location="right" content-class="mini-tooltip" open-delay="1000">
    <template #activator="{ props }">
      <v-btn
        v-bind="props"
        variant="text"
        :icon="icon"
        size="small"
        @click="$emit('click')"
      />
    </template>
    {{ tooltipText }}
  </v-tooltip>
  <v-tooltip location="right" content-class="mini-tooltip" open-delay="1000">
    <template #activator="{ props }">
      <v-expand-transition>
        <v-btn
          v-if="showChild"
          v-bind="props"
          variant="text"
          :icon="childIcon"
          size="small"
          @click="$emit('childClick')"
        />
      </v-expand-transition>
    </template>
    {{ childTooltipText }}
  </v-tooltip>
</template>

<script>
import { computed } from "vue";

export default {
  name: "WifiItemButton",
  props: {
    type: String,
    childType: String,
    showChild: Boolean,
  },
  emits: ["click", "childClick"],
  setup(props) {
    function getIcon(type) {
      if (type === "disconnect") return "mdi-logout";
      else if (type === "delete") return "mdi-delete";
      else if (type === "connect") return "mdi-check";
      else if (type === "options")
        return props.showChild ? "mdi-close" : "mdi-dots-vertical";
      else if (type === "pointer") return "mdi-button-pointer";
    }

    function getTooltipText(type) {
      if (type === "disconnect") return "Desconectar";
      else if (type === "delete") return "Remover";
      else if (type === "connect") return "Conectar";
      else if (type === "options")
        return props.showChild ? "Cancelar" : "Opções";
      return "Sem função definida";
    }

    const icon = computed(() => {
      return getIcon(props.type);
    });

    const childIcon = computed(() => {
      return getIcon(props.childType);
    });

    const tooltipText = computed(() => {
      return getTooltipText(props.type);
    });

    const childTooltipText = computed(() => {
      return getTooltipText(props.childType);
    });

    return {
      icon,
      tooltipText,
      childIcon,
      childTooltipText,
    };
  },
};
</script>
