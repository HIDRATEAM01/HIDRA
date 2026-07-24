#ifndef CAMERA_CONTROL_H
#define CAMERA_CONTROL_H

#include "esp_camera.h"
#include "fb_gfx.h"
#include "FS.h"
#include "SPIFFS.h" 

// Camera pins (AI Thinker)
#define PWDN_GPIO_NUM  32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM  0
#define SIOD_GPIO_NUM  26
#define SIOC_GPIO_NUM  27

#define Y9_GPIO_NUM    35
#define Y8_GPIO_NUM    34
#define Y7_GPIO_NUM    39
#define Y6_GPIO_NUM    36
#define Y5_GPIO_NUM    21
#define Y4_GPIO_NUM    19
#define Y3_GPIO_NUM    18
#define Y2_GPIO_NUM    5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM  23
#define PCLK_GPIO_NUM  22


class CameraControl {
  private:
    bool cameraOn;
  public:
    CameraControl() {
      cameraOn = false;
    }

    bool begin() {
      if (cameraOn) debug.println("[CAM] Camera ja iniciada. Reiniciando.");

      camera_config_t config;
      config.ledc_channel = LEDC_CHANNEL_0;
      config.ledc_timer = LEDC_TIMER_0;
      config.pin_d0 = Y2_GPIO_NUM;
      config.pin_d1 = Y3_GPIO_NUM;
      config.pin_d2 = Y4_GPIO_NUM;
      config.pin_d3 = Y5_GPIO_NUM;
      config.pin_d4 = Y6_GPIO_NUM;
      config.pin_d5 = Y7_GPIO_NUM;
      config.pin_d6 = Y8_GPIO_NUM;
      config.pin_d7 = Y9_GPIO_NUM;
      config.pin_xclk = XCLK_GPIO_NUM;
      config.pin_pclk = PCLK_GPIO_NUM;
      config.pin_vsync = VSYNC_GPIO_NUM;
      config.pin_href = HREF_GPIO_NUM;
      config.pin_sccb_sda = SIOD_GPIO_NUM;
      config.pin_sccb_scl = SIOC_GPIO_NUM;
      config.pin_pwdn = PWDN_GPIO_NUM;
      config.pin_reset = RESET_GPIO_NUM;
      config.xclk_freq_hz = 20000000;
      config.frame_size = FRAMESIZE_UXGA;
      config.pixel_format = PIXFORMAT_JPEG;

      config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
      config.fb_location = CAMERA_FB_IN_PSRAM;
      config.jpeg_quality = 12;
      config.fb_count = 1;

      if (psramFound()) {
        config.jpeg_quality = 10;
        config.fb_count = 2;
        config.grab_mode = CAMERA_GRAB_LATEST;
      } else {
        config.frame_size = FRAMESIZE_SVGA;
        config.fb_location = CAMERA_FB_IN_DRAM;
      }

      // Inicializar câmera
      esp_err_t err = esp_camera_init(&config);
      if (err != ESP_OK) {
        debug.println("[CAM] Erro: " + String(err));
        return false;
      }

      sensor_t *s = esp_camera_sensor_get();

      // initial sensors are flipped vertically and colors are a bit saturated
      if (s->id.PID == OV3660_PID) {
        s->set_vflip(s, 1);        // flip it back
        s->set_brightness(s, 1);   // up the brightness just a bit
        s->set_saturation(s, -2);  // lower the saturation
      }
      // drop down frame size for higher initial frame rate
      s->set_framesize(s, FRAMESIZE_QVGA);

      //applyDefaultSettings();
      cameraOn = true;
      debug.println("[CAM] Camera iniciada.");
      return true;
    }

    bool capture(String filename) {
      if (!cameraOn) {
        debug.println("[CAM] Iniciando camera.");
        begin();
        delay(500);
      }
      // Dispose first pictures because of bad quality
      camera_fb_t* fb = NULL;
      // Skip first 3 frames (increase/decrease number as needed).
      for (int i = 0; i < 3; i++) {
        fb = esp_camera_fb_get();
        esp_camera_fb_return(fb);
        fb = NULL;
      }
        
      // Take a new photo
      fb = NULL;  
      fb = esp_camera_fb_get();  
      if (!fb) {
        debug.println("[CAM] Falha ao capturar frame");
        return false;
      } 

      if (saveToFile(filename.c_str(), fb)) {
        debug.println("[CAM] Foto salva em " + filename);
      } else {
        debug.println("[CAM] Falha ao salvar foto");
        return false;
      }

      esp_camera_fb_return(fb);
      delay(100);
      return true;
    }

    bool saveToFile(const char* path, camera_fb_t* fb) {
      if (!SPIFFS.begin(true)) {
        return false;
      }

      File file = SPIFFS.open(path, FILE_WRITE);
      if (!file) {
          debug.println("[CAM] Falha ao abrir arquivo para escrita");
          return false;
      }

      size_t written = file.write(fb->buf, fb->len);
      file.close();

      if (written != fb->len) {
          debug.println("[CAM] Erro: Escreveu " + String(written) + " de " + String(fb->len));
          return false;
      }

      debug.println("[CAM] Arquivo salvo: " + String(path) + " (" + String(written) + " bytes)");
      return true;
  }

  void applyDefaultSettings() {
    sensor_t * s = esp_camera_sensor_get();
    if (!s) {
        debug.println("[Camera] Erro ao obter sensor");
        return;
    }

    // Configurações baseadas no painel
    s->set_brightness(s, 0);             // brilho
    s->set_contrast(s, 0);               // contraste
    s->set_saturation(s, 0);             // saturação
    s->set_special_effect(s, 0);         // sem efeito

    s->set_awb_gain(s, 1);               // AWB Gain ON
    s->set_whitebal(s, 1);               // AWB ON
    s->set_wb_mode(s, 0);                // Auto WB

    s->set_aec2(s, 0);                   // AEC DSP OFF
    s->set_ae_level(s, 0);               // AE Level 0
    s->set_gain_ctrl(s, 1);              // AGC ON
    s->set_gainceiling(s, (gainceiling_t)0); // 2x

    s->set_bpc(s, 0);                    // BPC OFF
    s->set_wpc(s, 1);                    // WPC ON
    s->set_raw_gma(s, 1);                // Raw GMA ON
    s->set_lenc(s, 1);                   // Lens Correction ON

    s->set_hmirror(s, 0);                // H-Mirror OFF
    s->set_vflip(s, 0);                  // V-Flip OFF
    s->set_dcw(s, 1);                    // DCW ON
    s->set_colorbar(s, 0);               // Color Bar OFF

    debug.println("[Camera] Configurações padrão aplicadas");
}


};

#endif
