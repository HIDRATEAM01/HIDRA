<template>
  <v-text-field
    :model-value="modelValue"
    @update:model-value="$emit('update:modelValue', $event)"
    :type="type"
    :step="step"
    :label="field.label"
    :append-inner-icon="innerIcon"
    variant="outlined"
    hide-details="auto"
    class="mb-3 text-main"
    density="comfortable"
    :disabled="readonly"
    @click:append-inner="showpass = !showpass"
  />
</template>

<script>
import { ref, computed, watch } from "vue";

export default {
  name: "ConfigTextField",
  props: {
    modelValue: [String, Number, Boolean],
    field: Object,
    readonly: Boolean,
  },
  emits: ["update:modelValue"],
  setup(props, { emit }) {
    const showpass = ref(false);

    watch(
      () => props.readonly,
      (newValue) => {
        if (newValue && props.field.type === "password") {
          showpass.value = false;
        }
      }
    );

    const innerIcon = computed(() => {
      if (props.field.type === "password") {
        return showpass.value ? "mdi-eye" : "mdi-eye-off";
      }
      return undefined;
    });

    const step = computed(() => {
      return props.field.type === "time" ? "1" : undefined;
    });

    const type = computed(() => {
      if (props.field.type === "password") {
        return showpass.value ? "text" : "password";
      }
      return props.field.type || "text";
    });

    return {
      emit,
      showpass,
      innerIcon,
      step,
      type,
    };
  },
};
</script>
