import api from "./axios";

export async function getConfig() {
  return await api.get("/config");
}

export async function postConfigTime(config) {
  return await api.post("/config/time", config, {
    headers: {
      "Content-Type": "application/json",
    },
  });
}
