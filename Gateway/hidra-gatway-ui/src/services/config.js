import api from "./axios";

export async function getConfig() {
  return await api.get("/config");
}
