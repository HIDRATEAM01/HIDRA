import api from "./axios";

export function getWifiStatus() {
  return api.get("/wifi/status");
}

export function getWifiNetworks() {
  return api.get("/wifi/networks");
}

export function getServerConfig() {
  return api.get("/server/config");
}
