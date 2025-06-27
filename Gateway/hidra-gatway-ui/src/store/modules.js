import { defineStore } from "pinia";

export const useModuleStore = defineStore("modules", {
  state: () => ({
    valor: 0,
  }),
  actions: {},
});
