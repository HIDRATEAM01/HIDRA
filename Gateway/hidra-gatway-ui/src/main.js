import { createApp } from "vue";
import App from "./App.vue";
import router from "./router";
import { createPinia } from "pinia";
import vuetify from "./plugins/vuetify";

const pinia = createPinia();

createApp(App).use(pinia).use(router).use(vuetify).mount("#app");
