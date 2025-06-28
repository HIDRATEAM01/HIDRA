<template>
  <v-card position="relative" style="width: 350px">
    <v-card-title class="mr-3">
      <v-icon v-if="icon" :icon="icon" size="medium" class="mr-2 mb-1" />
      {{ title }}
    </v-card-title>
    <v-card-text>
      <v-form>
        <ConfigTextField
          v-for="field in fields"
          :key="field.name"
          v-model="field.model"
          :field="field"
          :readonly="readonly"
        />
        <v-row class="px-10">
          <v-col v-if="primaryButton">
            <v-btn
              color="primary"
              class="text-none text-subtitle-1"
              @click="$emit('primaryClick')"
            >
              {{ primaryButton }}
            </v-btn>
          </v-col>
          <v-col v-if="secondaryButton">
            <v-btn
              color="secondary"
              class="text-none text-subtitle-1"
              @click="$emit('secondaryClick')"
            >
              {{ secondaryButton }}
            </v-btn>
          </v-col>
        </v-row>
      </v-form>
    </v-card-text>
    <v-overlay
      :model-value="loading"
      class="align-center justify-center"
      contained
      persistent
    >
      <v-progress-circular indeterminate color="primary" size="64" />
      <div class="mt-4">Carregando...</div>
    </v-overlay>
  </v-card>
</template>

<script>
import ConfigTextField from "@/components/ConfigTextField.vue";

export default {
  name: "ConfigBlock",
  components: {
    ConfigTextField,
  },
  props: {
    icon: String,
    title: String,
    fields: Array,
    loading: Boolean,
    primaryButton: String,
    secondaryButton: String,
    readonly: Boolean,
  },
};
</script>
