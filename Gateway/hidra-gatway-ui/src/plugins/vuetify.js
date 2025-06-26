import "@mdi/font/css/materialdesignicons.css";
import { createVuetify } from "vuetify";
import { aliases, mdi } from "vuetify/iconsets/mdi";
import * as components from "vuetify/components";
import * as directives from "vuetify/directives";
import "vuetify/styles";

export default createVuetify({
  components,
  directives,
  icons: {
    defaultSet: "mdi",
    aliases,
    sets: {
      mdi,
    },
  },
  theme: {
    defaultTheme: "principal",
    themes: {
      principal: {
        dark: true,
        colors: {
          background: "#373d42",
          surface: "#43494f",
          primary: "#4c629a",
          secondary: "#494e54",
          textmain: "#FFFFFF",
          textsec: "#e5e8e7",
          accent: "#dce2e3",
          hover: "#3F3F3F",
        },
      },
    },
  },
});
