import api from "./axios";

export function getModules() {
  return api.get("/modules");
}
