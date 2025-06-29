import { createVuetify } from "vuetify";
import { h } from "vue";
import * as components from "vuetify/components";
import * as directives from "vuetify/directives";
import "vuetify/styles";

import {
  mdiLan,
  mdiWifi,
  mdiCog,
  mdiCheck,
  mdiEye,
  mdiEyeOff,
  mdiPencil,
  mdiClose,
  mdiRefresh,
  mdiWifiRemove,
  mdiWifiStrengthOutline,
  mdiWifiStrength1,
  mdiWifiStrength2,
  mdiWifiStrength3,
  mdiWifiStrength4,
  mdiLogout,
  mdiDelete,
  mdiDotsVertical,
  mdiButtonPointer,
} from "@mdi/js";

const customIcons = {
  "mdi-lan": mdiLan,
  "mdi-wifi": mdiWifi,
  "mdi-cog": mdiCog,
  "mdi-check": mdiCheck,
  "mdi-eye": mdiEye,
  "mdi-eye-off": mdiEyeOff,
  "mdi-pencil": mdiPencil,
  "mdi-close": mdiClose,
  "mdi-refresh": mdiRefresh,
  "mdi-wifi-remove": mdiWifiRemove,
  "mdi-wifi-strength-outline": mdiWifiStrengthOutline,
  "mdi-wifi-strength-1": mdiWifiStrength1,
  "mdi-wifi-strength-2": mdiWifiStrength2,
  "mdi-wifi-strength-3": mdiWifiStrength3,
  "mdi-wifi-strength-4": mdiWifiStrength4,
  "mdi-logout": mdiLogout,
  "mdi-delete": mdiDelete,
  "mdi-dots-vertical": mdiDotsVertical,
  "mdi-button-pointer": mdiButtonPointer,
};

export default createVuetify({
  components,
  directives,
  icons: {
    defaultSet: "mdi",
    sets: {
      mdi: {
        component: (props) =>
          h(
            "svg",
            {
              class: "v-icon__svg",
              xmlns: "http://www.w3.org/2000/svg",
              viewBox: "0 0 24 24",
              fill: "currentColor",
              style: {
                width: "1em",
                height: "1em",
              },
            },
            [h("path", { d: customIcons[props.icon] })]
          ),
      },
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
