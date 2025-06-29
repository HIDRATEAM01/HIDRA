module.exports = {
  transpileDependencies: true,
  filenameHashing: false,
  assetsDir: "",
  productionSourceMap: false,

  chainWebpack: (config) => {
    // Ajusta o loader das imagens para gerar na raiz sem hash
    config.module.rule("images").set("generator", {
      filename: "[name][ext]",
    });
  },

  configureWebpack: {
    output: {
      filename: "[name].js",
      chunkFilename: "[name].js",
    },
  },

  css: {
    extract: {
      filename: "[name].css",
      chunkFilename: "[name].css",
    },
  },
};
