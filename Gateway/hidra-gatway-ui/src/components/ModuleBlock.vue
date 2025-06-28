<template>
  <v-card class="px-3" style="width: 350px">
    <v-card-title class="mr-3 mb-4">
      <v-row justify="space-between">
        <v-col class="ml-16">
          {{ props.title }}
        </v-col>
        <v-col cols="2">
          <v-btn icon variant="text" density="compact">
            <v-icon icon="mdi-pencil" size="small" />
          </v-btn>
        </v-col>
      </v-row>
    </v-card-title>
    <v-card-text>
      <v-row v-for="(field, key) in fields" :key="key">
        <v-text-field
          :label="field.label"
          v-model="field.value"
          variant="outlined"
          hide-details="auto"
          class="mb-3 text-end"
          density="comfortable"
          disabled
        />
      </v-row>
      <v-row>
        <v-col>
          <v-btn color="primary" class="text-none text-subtitle-1">
            Ver Medições
          </v-btn>
        </v-col>
      </v-row>
    </v-card-text>
  </v-card>
</template>

<script>
export default {
  name: "ModuleBlock",
  props: {
    title: String,
    values: Object,
  },
  setup: function (props) {
    const fields = [
      {
        label: "Data/Hora",
        value: props.values.recieveTime,
      },
      { label: "Bateria", value: `${props.values.bat} %` },
      { label: "Endereço", value: props.values.address },
    ];

    return { props, fields };
  },
};
</script>
