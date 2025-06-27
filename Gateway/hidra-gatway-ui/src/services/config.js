import api from "./axios";

export async function getConfig() {
  return await api.get("/config");
}
//TODO: Add store function to save new server config
export async function postConfigTime(config) {
  return await api.post("/config/time", config);
}
