import api from "./axios";

export function getNetworks() {
  return api.get("/networks");
}
