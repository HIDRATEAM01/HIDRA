#include "utils.h"

Utils utils;

int Utils::inLimit(int value, int min, int max) {
  if (value < min)
    return min;
  else if (value > max)
    return max;
  else
    return value;
}

float Utils::toFixed(float value, int precision) {
  int factor = pow(10, precision);
  return round(value * factor) / factor;
}

String Utils::ip2Str(IPAddress ip) {
  String s = "";
  for (int i = 0; i < 4; i++) {
    s += i ? "." + String(ip[i]) : String(ip[i]);
  }
  return s;
}

String Utils::formatBytes(size_t bytes) {
  if (bytes < 1024)
    return String(bytes) + "B";
  else if (bytes < (1024 * 1024))
    return String(bytes / 1024.0) + "KB";
  else
    return String(bytes / (1024.0 * 1024.0)) + "MB";
}

String Utils::getContentType(String filename) {
  if (filename.endsWith(".htm") || filename.endsWith(".html"))
    return "text/html";
  else if (filename.endsWith(".css"))
    return "text/css";
  else if (filename.endsWith(".csv"))
    return "text/csv";
  else if (filename.endsWith(".js"))
    return "application/javascript";
  else if (filename.endsWith(".json"))
    return "application/json";
  else if (filename.endsWith(".xml"))
    return "text/xml";

  else if (filename.endsWith(".png"))
    return "image/png";
  else if (filename.endsWith(".gif"))
    return "image/gif";
  else if (filename.endsWith(".jpg") || filename.endsWith(".jpeg"))
    return "image/jpeg";
  else if (filename.endsWith(".svg"))
    return "image/svg+xml";
  else if (filename.endsWith(".ico"))
    return "image/x-icon";

  else if (filename.endsWith(".pdf"))
    return "application/pdf";
  else if (filename.endsWith(".zip"))
    return "application/zip";
  else if (filename.endsWith(".gz"))
    return "application/gzip";

  else if (filename.endsWith(".mp3"))
    return "audio/mpeg";
  else if (filename.endsWith(".wav"))
    return "audio/wav";
  else if (filename.endsWith(".mp4"))
    return "video/mp4";

  else if (filename.endsWith(".woff"))
    return "font/woff";
  else if (filename.endsWith(".woff2"))
    return "font/woff2";
  else if (filename.endsWith(".ttf"))
    return "font/ttf";
  else if (filename.endsWith(".otf"))
    return "font/otf";

  return "text/plain";
}
