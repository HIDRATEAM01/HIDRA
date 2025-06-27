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
      <v-row v-for="(label, key) in labels" :key="key">
        <v-text-field
          :label="label"
          variant="outlined"
          hide-details="auto"
          class="mb-3 text-end"
          density="comfortable"
          readonly
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
  },
  setup: function (props) {
    const labels = {
      leitura: "Data/Hora",
      relogio: "Bateria",
      bateria: "Endereço",
    };
    return { props, labels };
  },
};
</script>
