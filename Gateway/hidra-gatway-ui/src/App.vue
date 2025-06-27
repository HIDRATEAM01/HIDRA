<template>
  <router-view />
  <AppSnackbar />
</template>

<script>
import AppSnackbar from "@/components/SnackBar.vue";

export default {
  components: {
    AppSnackbar,
  },
};
</script>

<style>
html,
body,
#app {
  font-family: Avenir, Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  text-align: center;
}

.shadow-2 {
  text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.6);
}
</style>
