<template>
  <v-card>
    <v-card-title class="mr-3">
      <v-icon v-if="icon" :icon="icon" size="medium" class="mr-2 mb-1" />
      {{ title }}
    </v-card-title>
    <v-card-text>
      <v-form>
        <v-text-field
          v-for="field in fields"
          :key="field.name"
          v-model="formData[field.name]"
          :label="field.label"
          :type="field.type || 'text'"
          variant="outlined"
          hide-details="auto"
          class="mb-3 text-end"
        />
        <v-row>
          <v-col v-if="primaryButton">
            <v-btn
              color="primary"
              class="text-none text-subtitle-1"
              @click="$emit('submit', formData)"
            >
              {{ primaryButton }}
            </v-btn>
          </v-col>
          <v-col v-if="secondaryButton">
            <v-btn
              color="secondary"
              class="text-none text-subtitle-1 font-weight-800"
              @click="$emit('submit', formData)"
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
import { toRefs, reactive } from "vue";

export default {
  name: "ConfigBlock",
  props: {
    icon: String,
    title: String,
    fields: Array,
    primaryButton: String,
    secondaryButton: String,
  },
  setup: function (props) {
    const formData = reactive({});

    props.fields.forEach((field) => {
      formData[field.name] = field.value || "";
    });

    return {
      ...toRefs(props),
      formData,
    };
  },
};
</script>
