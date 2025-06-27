/* eslint-disable no-unused-vars */
import axios from "axios";

const testURL = "http://192.168.100.69:8000";
const productionURL = "";

const api = axios.create({
  baseURL: testURL,
  timeout: 10000,
});

export default api;
