import axios from "axios";

const api = axios.create({
  baseURL: process.env.VUE_APP_BASE_URL,
  timeout: 10000,
});

export default api;
