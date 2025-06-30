import api from "./axios";

export function getServerConfig() {
  return api.get("/server/config");
}

export function postServerConfig(config) {
  return api.post("/server/config", config, {
    headers: {
      "Content-Type": "application/json",
    },
  });
}

export function postServerStatus(config) {
  return api.post("/server/toggle", config, {
    headers: {
      "Content-Type": "application/json",
    },
  });
}
