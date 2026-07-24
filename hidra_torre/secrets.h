#ifndef SECRETS_H
#define SECRETS_H

#define WIFI_SSID ""
#define WIFI_PASS ""

#define PHONE_SSID ""
#define PHONE_PASS ""

#define DEF_SSID ""
#define DEF_PASS ""

// #define TESTE_FIREBASE

#if defined(TESTE_FIREBASE)
#define FIREBASE_EMAIL ""
#define FIREBASE_PASS ""
#define FIREBASE_BUCKET ""
#define FIREBASE_RTDB ""
#define FIREBASE_API ""
#else
#define FIREBASE_API ""
#define FIREBASE_EMAIL ""
#define FIREBASE_PASS ""
#define FIREBASE_RTDB ""
#define FIREBASE_BUCKET ""
#endif

#endif