import api from "./axios";

export function getWifiStatus() {
  return api.get("/wifi/status");
}

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
