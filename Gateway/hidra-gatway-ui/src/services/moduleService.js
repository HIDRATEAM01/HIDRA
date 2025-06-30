import api from "./axios";

export function getModules() {
  return api.get("/modules");
}

// TODO: Implement
export function getModuleById(module) {
  return api.get(`/modules/${module}/data`);
}

//TODO: Implement
export function postModuleConfig(idmodule, config) {
  return api.post(`/modules/${idmodule}/config`, config);
}

//TODO: Implement
export function postModuleNew(config) {
  return api.post(`/modules/new`, config);
}

//TODO: Implement
export function deleteModule(idmodule, config) {
  return api.delete(`/modules/${idmodule}`, config);
}
