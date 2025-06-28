import api from "./axios";

export function getWifiStatus() {
  return api.get("/wifi/status");
}

//TODO: Add store function to get the current wifi network list
export function getWifiNetworks() {
  return api.get("/wifi/networks");
}

export function postWifiStatus(config) {
  return api.post("/wifi/toggle", config, {
    headers: {
      "Content-Type": "application/json",
    },
  });
}

//TODO: Add store function to save the current wifi network
export function postWifiConnect(config) {
  return api.post("/wifi/connect", config, {
    headers: {
      "Content-Type": "application/json",
    },
  });
}

//TODO: Add store function to delete the current wifi network
export function deleteWifiNetwork(networkId, config) {
  return api.delete(`/wifi/networks/${networkId}`, config);
}
