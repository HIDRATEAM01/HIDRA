import api from "./axios";

//TODO: ENTIRE FILE
export function getModules() {
  return api.get("/modules");
}
