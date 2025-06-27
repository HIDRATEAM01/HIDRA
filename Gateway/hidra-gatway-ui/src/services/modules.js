import api from "./axios";

export function getModules() {
  return api.get("/modules");
}

// TODO: Implement pagination
export function getModule(module) {
  return api.get(`/modules/${module}/data`);
}

//TODO: Implement pagination
export function postModuleConfig(idmodule, config) {
  return api.post(`/modules/${idmodule}/config`, config);
}

//TODO: Implement pagination
export function postModuleNew(config) {
  return api.post(`/modules/new`, config);
}

//TODO: Implement pagination
export function deleteModule(idmodule, config) {
  return api.delete(`/modules/${idmodule}`, config);
}
