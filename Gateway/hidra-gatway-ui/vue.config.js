const webpack = require("webpack");

module.exports = {
  transpileDependencies: true,
  filenameHashing: false,
  assetsDir: "",
  productionSourceMap: false,

  chainWebpack: (config) => {
    config.module.rule("images").set("generator", {
      filename: "[name][ext]",
    });
  },

  pluginOptions: {
    compressionOptions: {
      algorithm: "gzip",
      ext: ".gz",
      treshold: 0,
      deleteOriginalAssets: false,
      test: /\.(js|css|html|svg)$/,
    },
  },

  configureWebpack: {
    output: {
      filename: "[name].js",
      chunkFilename: "[name].js",
    },
    plugins: [
      new webpack.DefinePlugin({
        __VUE_PROD_HYDRATION_MISMATCH_DETAILS__: "false",
      }),
    ],
  },

  css: {
    extract: {
      filename: "[name].css",
      chunkFilename: "[name].css",
    },
  },
};
