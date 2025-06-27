import api from "./axios";

export function getWifiStatus() {
  return api.get("/wifi/status");
}

//TODO: Add store function to get the current wifi network list
export function getWifiNetworks() {
  return api.get("/wifi/networks");
}

export function getServerConfig() {
  return api.get("/server/config");
}

//TODO: Add store function to save the current wifi network
export function postWifiConnect() {
  return api.post("/wifi/connect");
}

//TODO: Add store function to save new server config
export function postServerConfig(config) {
  return api.post("/server/config", config);
}

//TODO: Add store function to delete the current wifi network
export function deleteWifiNetwork(networkId, config) {
  return api.delete(`/wifi/networks/${networkId}`, { data: config });
}
