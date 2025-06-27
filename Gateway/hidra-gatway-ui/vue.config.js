const { defineConfig } = require("@vue/cli-service");
module.exports = defineConfig({
  transpileDependencies: true,
  define: {
    __VUE_PROD_HYDRATION_MISMATCH_DETAILS__: true,
  },
});
