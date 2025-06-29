#include "file_manager.h"

FileManager::FileManager() {}

bool FileManager::begin() {
  if (!FILE_SYSTEM.begin(true)) {
    serial.log(LOG_ERROR, "[FileManager] Falha ao montar sistema de arquivos.");
    return false;
  }
  serial.log(LOG_INFO, "[FileManager] Sistema de arquivos montado.");

  serial.log(LOG_INFO, "[FileManager] Tamanho Flash: ", utils.formatBytes(ESP.getFlashChipSize()).c_str());
  serial.log(LOG_INFO, "[FileManager] Tamanho Sketch: ", utils.formatBytes(ESP.getSketchSize()).c_str());
  serial.log(LOG_INFO, "[FileManager] tamanho do sistema de arquivos: ", utils.formatBytes(FILE_SYSTEM.totalBytes()).c_str());
  serial.log(LOG_INFO, "[FileManager] Total utilizado: ", utils.formatBytes(FILE_SYSTEM.usedBytes()).c_str());

  return true;
}

bool FileManager::createFile(const String &path) {
  serial.log(LOG_INFO, "[FileManager] Criando arquivo: ", path.c_str());
  File file = FILE_SYSTEM.open(path, "w");
  if (!file) return false;
  file.close();
  return true;
}

bool FileManager::saveFile(const char *path, const uint8_t *data, size_t length) {
  File file = FILE_SYSTEM.open(path, "w");
  if (!file)
    return false;
  file.write(data, length);
  file.close();
  return true;
}

bool FileManager::readFile(const char *path, uint8_t *buffer, size_t bufferSize) {
  File file = FILE_SYSTEM.open(path, "r");
  if (!file)
    return false;
  file.read(buffer, bufferSize);
  file.close();
  return true;
}

bool FileManager::writeToFile(const String &path, const uint8_t *data, size_t len) {
  File file = FILE_SYSTEM.open(path, "a");
  if (!file) return false;
  file.write(data, len);
  file.close();
  return true;
}

bool FileManager::exists(const String path) {
  return FILE_SYSTEM.exists(path);
}

File FileManager::getFile(const char *path, const char *mode) {
  File file = FILE_SYSTEM.open(path, mode);

  if (!file || !file.available()) {
    serial.log(LOG_ERROR, "[FileManager] Arquivo não encontrado: ", path);
    return File();
  }

  return file;
}

void FileManager::listFiles(const char *path, int maxDepth) {
  File root = FILE_SYSTEM.open(path);
  if (!root || !root.isDirectory()) {
    serial.log(LOG_ERROR, "[FileManager] Diretório inválido: ", path);
    return;
  }

  while (File file = root.openNextFile()) {
    String fullPath = String(path);
    if (!fullPath.endsWith("/")) {
      fullPath += "/";
    }
    fullPath += file.name();

    if (file.isDirectory()) {
      serial.log(LOG_INFO, "[FileManager][DIR] ", fullPath.c_str());

      if (maxDepth > 0) {
        listFiles(fullPath.c_str(), maxDepth - 1);
      }
    } else {
      serial.log(LOG_INFO, "[FileManager][FILE]: ", fullPath.c_str(), " - ", utils.formatBytes(file.size()).c_str());
    }
    file.close();
  }
}

bool FileManager::removeRecursive(const char *path) {
  File root = FILE_SYSTEM.open(path);
  if (!root) {
    serial.log(LOG_ERROR, "[FileManager] Falha ao abrir diretório: ", path);
    return false;
  }

  if (!root.isDirectory()) {
    root.close();
    serial.log(LOG_INFO, "[FileManager] Removendo arquivo: ", path);
    return FILE_SYSTEM.remove(path);
  }

  serial.log(LOG_INFO, "[FileManager] Removendo diretório existente: ", path);

  File file = root.openNextFile();
  while (file) {
    String filePath = String(path) + "/" + file.name();

    if (file.isDirectory()) {
      if (!removeRecursive(filePath.c_str())) {
        serial.log(LOG_ERROR, "[FileManager] Falha ao remover subdiretório: ", filePath.c_str());
        file.close();
        root.close();
        return false;
      }
    } else {
      file.close();
      if (!FILE_SYSTEM.remove(filePath)) {
        serial.log(LOG_ERROR, "[FileManager] Falha ao remover arquivo: ", filePath.c_str());
        root.close();
        return false;
      }
    }

    file = root.openNextFile();
  }

  root.close();
  return FILE_SYSTEM.rmdir(path);
}

void FileManager::end() {
  FILE_SYSTEM.end();
}
