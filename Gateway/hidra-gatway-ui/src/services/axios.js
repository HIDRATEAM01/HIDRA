import axios from "axios";

//const baseURL = "http://localhost:8000";
const baseURL = "";

const api = axios.create({
  baseURL: baseURL,
  timeout: 10000,
});

export default api;
