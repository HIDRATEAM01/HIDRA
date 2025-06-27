<template>
  <v-card style="width: 350px">
    <v-card-title class="mr-3">
      <v-icon v-if="icon" :icon="icon" size="medium" class="mr-2 mb-1" />
      {{ title }}
    </v-card-title>
    <v-card-text>
      <v-form>
        <v-text-field
          v-for="field in fields"
          :key="field.name"
          v-model="model[field.name].value"
          :label="field.label"
          :type="field.type || 'text'"
          :step="field.type === 'time' ? '1' : undefined"
          variant="outlined"
          hide-details="auto"
          class="mb-3 text-main"
          density="comfortable"
          :disabled="readonly"
        />
        <v-row class="px-10">
          <v-col v-if="primaryButton">
            <v-btn
              color="primary"
              class="text-none text-subtitle-1"
              @click="$emit('primaryClick', formData)"
            >
              {{ primaryButton }}
            </v-btn>
          </v-col>
          <v-col v-if="secondaryButton">
            <v-btn
              color="secondary"
              class="text-none text-subtitle-1"
              @click="$emit('secondaryClick', formData)"
            >
              {{ secondaryButton }}
            </v-btn>
          </v-col>
        </v-row>
      </v-form>
    </v-card-text>
  </v-card>
</template>

<script>
export default {
  name: "ConfigBlock",
  props: {
    icon: String,
    title: String,
    fields: Array,
    primaryButton: String,
    secondaryButton: String,
    formModel: Object,
    readonly: Boolean,
  },

  setup(props) {
    const model = props.formModel || {};

    props.fields.forEach((field) => {
      if (!(field.name in model)) {
        model[field.name] = "";
      }
    });

    return { model };
  },
};
</script>
