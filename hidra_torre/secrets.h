#ifndef SECRETS_H
#define SECRETS_H

#define WIFI_SSID "SOFTLINK_ERIC"
#define WIFI_PASS "98663197"

#define PHONE_SSID "Redmi Note 14 Pro"
#define PHONE_PASS "eric1234"

#define DEF_SSID "HIDRA1"
#define DEF_PASS "hidra123"

//#define TESTE_FIREBASE

#if defined(TESTE_FIREBASE)
#define FIREBASE_EMAIL "ericlucas01@gmail.com"
#define FIREBASE_PASS "SenhaEricHidraTeste"
#define FIREBASE_BUCKET "hidra-6feb7.firebasestorage.app"
#define FIREBASE_RTDB "hidra-6feb7-default-rtdb.firebaseio.com"
#define FIREBASE_API "AIzaSyDuITLvXqreunOn2HanAGgJrhaD4puckJ0"
#else
#define FIREBASE_API "AIzaSyDoDunY6M3QYgOcMjYBDykvkmVI6xpTqko"
#define FIREBASE_EMAIL "esp32-cam-wokwi@hidra-eco.com"
#define FIREBASE_PASS "Hidra14789632"
#define FIREBASE_RTDB "hidra-eco-default-rtdb.firebaseio.com"
#define FIREBASE_BUCKET "hidra-eco.firebasestorage.app" 
#endif

#endif