/* eslint-disable no-unused-vars */
import axios from "axios";

const testURL = "http://localhost:8000";
const productionURL = "";

const api = axios.create({
  baseURL: testURL,
  timeout: 10000,
});

export default api;
