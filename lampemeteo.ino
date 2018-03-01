// Wifi Lamp Project
// Steve Olmstead 10/02/2018
// Based on work from Vagtsal, 14/7/2017
#include "esp_system.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <FastLED.h>
FASTLED_USING_NAMESPACE
#include <Preferences.h>
Preferences preferences;
#define HOURS_TO_REQUEST_WEATHER 1
String openWeatherID = "b26028bf0fdc805144238f7962b46307";
String googleMapsID = "AIzaSyALE5CnnsnrJDq6YMSaolVtGGQAPHT316g";
unsigned long timerd = 0;
String latitude;
String longitude;
String forecast;
String meteo;
String msg;
String derncoul = "";
#define LED_PIN1    13
#define COLOR_ORDER GRB
#define CHIPSET     WS2812B
#define NUM_LEDS    18
#define NUM_LEDS2   108
#define COOLING  55
#define SPARKING 65
int FRAMES_PER_SECOND = 80;
CRGB leds1[NUM_LEDS2];
CRGB color;
int effect;
int brightness;
int weather;
int NO_EFFECT  =  0;
int FIRE       =  1;
int WEATHER    =  2;
int CLEAR      =  1;
int CLOUDS     =   2;
int RAIN       =   3;
int THUNDERSTORM = 4;
int SNOW        =  5;
WebServer serveur(80);
int rouge = 0;
int vert = 0;
int bleu = 0;
int runshow = 0;
int change = 1;
unsigned long deuxMillis = 0;
const long interval = 5000;
unsigned int manageur = 0;

void configModeCallback (WiFiManager *myWiFiManager) {
  manageur = 1;
}

// convert string 2 char*
char* string2char(String command) {
  if (command.length() != 0) {
    char *p = const_cast<char*>(command.c_str());
    return p;
  }
}

void change_state(String choice) {
  if (choice == "r\n") {
    rouge = color.r;
    preferences.putUInt("RED_ADDR", color.r);
  }
  else if (choice == "g\n") {
    vert = color.g;
    preferences.putUInt("GREEN_ADDR", color.g);
  }
  else if (choice == "b\n") {
    bleu = color.b;
    preferences.putUInt("BLUE_ADDR", color.b);
  }
  else if (choice == "bu\n") {
    if (brightness <= 75) {
      brightness += 25;
    } else {
      brightness = 100;
    }
    preferences.putUInt("BRIGHT_ADDR", brightness);
  }
  else if (choice == "bd\n") {
    if (brightness >= 25) {
      brightness -= 25;
    } else {
      brightness = 0;
    }
    preferences.putUInt("BRIGHT_ADDR", brightness);
  }
  else if (choice == "f\n") {
    effect = FIRE;
    preferences.putUInt("EFFECT_ADDR", FIRE);
  }
  else if (choice == "nf\n") {
    effect = NO_EFFECT;
    preferences.putUInt("EFFECT_ADDR", NO_EFFECT);
  }
  else if (choice == "w\n") {
    effect = WEATHER;
    preferences.putUInt("EFFECT_ADDR", WEATHER);
  }
  else if (choice == "nw\n") {
    effect = NO_EFFECT;
    preferences.putUInt("EFFECT_ADDR", NO_EFFECT);
  }
  else if (choice == "ws\n") {
    weather = CLEAR;
    preferences.putUInt("WEATHER_ADDR", weather);
  }
  else if (choice == "wc\n") {
    weather = CLOUDS;
    preferences.putUInt("WEATHER_ADDR", weather);
  }
  else if (choice == "wr\n") {
    weather = RAIN;
    preferences.putUInt("WEATHER_ADDR", weather);
  }
  else if (choice == "wt\n") {
    weather = THUNDERSTORM;
    preferences.putUInt("WEATHER_ADDR", weather);
  }
  else if (choice == "wx\n") {
    weather = SNOW;
    preferences.putUInt("WEATHER_ADDR", weather);
  }
  if (effect == NO_EFFECT) {
    for ( int j = 0; j < NUM_LEDS2; j++) {
      leds1[j] = color;
    }
  }
  // exponential mapping
  runshow = 1;
}

static byte heat[NUM_LEDS];
static byte heat2[NUM_LEDS];
static byte heat3[NUM_LEDS];
static byte heat4[NUM_LEDS];
static byte heat5[NUM_LEDS];
static byte heat6[NUM_LEDS];

void Fire2012(CRGB leds[NUM_LEDS2]) {
  // Array of temperature readings at each simulation cell
  // Step 1.  Cool down every cell a little
  for ( int i = 0; i < NUM_LEDS; i++) {
    heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    heat2[i] = qsub8( heat2[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    heat3[i] = qsub8( heat3[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    heat4[i] = qsub8( heat4[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    heat5[i] = qsub8( heat5[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    heat6[i] = qsub8( heat6[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
  }
  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for ( int k = NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    heat2[k] = (heat2[k - 1] + heat2[k - 2] + heat2[k - 2] ) / 3;
    heat3[k] = (heat3[k - 1] + heat3[k - 2] + heat3[k - 2] ) / 3;
    heat4[k] = (heat4[k - 1] + heat4[k - 2] + heat4[k - 2] ) / 3;
    heat5[k] = (heat5[k - 1] + heat5[k - 2] + heat5[k - 2] ) / 3;
    heat6[k] = (heat6[k - 1] + heat6[k - 2] + heat6[k - 2] ) / 3;
  }
  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if ( random8() < SPARKING ) {
    int y = random8(7);
    heat[y] = qadd8( heat[y], random8(160, 255) );
  }
  if ( random8() < SPARKING ) {
    int y = random8(7);
    heat2[y] = qadd8( heat2[y], random8(160, 255) );
  }
  if ( random8() < SPARKING ) {
    int y = random8(7);
    heat3[y] = qadd8( heat3[y], random8(160, 255) );
  }
  if ( random8() < SPARKING ) {
    int y = random8(7);
    heat4[y] = qadd8( heat4[y], random8(160, 255) );
  }
  if ( random8() < SPARKING ) {
    int y = random8(7);
    heat5[y] = qadd8( heat5[y], random8(160, 255) );
  }
  if ( random8() < SPARKING ) {
    int y = random8(7);
    heat6[y] = qadd8( heat6[y], random8(160, 255) );
  }
  // Step 4.  Map from heat cells to LED colors
  for ( int j = 0; j < 18; j++) {
    CRGB color = HeatColor( heat[j]);
    leds[j] = color;
  }
  for ( int j = 0; j < 18; j++) {
    CRGB color = HeatColor( heat2[j]);
    int multi = 1 * NUM_LEDS;
    int fixed = j + multi;
    leds[fixed] = color;
  }
  for ( int j = 0; j < 18; j++) {
    CRGB color = HeatColor( heat3[j]);
    int multi = 2 * NUM_LEDS;
    int fixed = j + multi;
    leds[fixed] = color;
  }
  for ( int j = 0; j < 18; j++) {
    CRGB color = HeatColor( heat4[j]);
    int multi = 3 * NUM_LEDS;
    int fixed = j + multi;
    leds[fixed] = color;
  }
  for ( int j = 0; j < 18; j++) {
    CRGB color = HeatColor( heat5[j]);
    int multi = 4 * NUM_LEDS;
    int fixed = j + multi;
    leds[fixed] = color;
  }
  for ( int j = 0; j < 18; j++) {
    CRGB color = HeatColor( heat6[j]);
    int multi = 5 * NUM_LEDS;
    int fixed = j + multi;
    leds[fixed] = color;
  }
}

void weather_effect(CRGB leds[NUM_LEDS2], int maperiod0, int maperiod1, int maperiod2, int maperiod3, int maperiod4, int maperiod5) {
  for (int imagod = 0; imagod < 6; imagod++) {
    int multi = imagod * NUM_LEDS;
    int period0, period1, period2, period3, period4, period5;
    switch (imagod) {
      case 0:
        period0 = maperiod0;
        period1 = maperiod1;
        period2 = maperiod2;
        period3 = maperiod3;
        period4 = maperiod4;
        period5 = maperiod4;
        break;
      case 1:
        period0 = maperiod1;
        period1 = maperiod2;
        period2 = maperiod3;
        period3 = maperiod4;
        period4 = maperiod5;
        period5 = maperiod0;
        break;
      case 2:
        period0 = maperiod2;
        period1 = maperiod3;
        period2 = maperiod4;
        period3 = maperiod5;
        period4 = maperiod0;
        period5 = maperiod1;
        break;
      case 3:
        period0 = maperiod3;
        period1 = maperiod4;
        period2 = maperiod5;
        period3 = maperiod0;
        period4 = maperiod1;
        period5 = maperiod2;
        break;
      case 4:
        period0 = maperiod4;
        period1 = maperiod5;
        period2 = maperiod0;
        period3 = maperiod1;
        period4 = maperiod2;
        period5 = maperiod3;
        break;
      case 5:
        period0 = maperiod5;
        period1 = maperiod0;
        period2 = maperiod1;
        period3 = maperiod2;
        period4 = maperiod3;
        period5 = maperiod4;
        break;
    }
    for ( int i = 0; i < NUM_LEDS; i++) {
      int fixed = i + multi;
      switch (i) {
        case 0:
        case 5:
        case 2:
          if (weather == CLEAR || weather == CLOUDS) {
            leds[fixed].g = period0;
          }
          if (weather == CLOUDS) {
            leds[fixed].r = period0;
          }
          break;
        case 1:
        case 17:
        case 15:
          if (weather == CLEAR || weather == CLOUDS) {
            leds[fixed].g = period1;
          }
          if (weather == CLOUDS) {
            leds[fixed].r = period1;
          }
          break;
        case 12:
        case 7:
        case 10:
          if (weather == CLEAR || weather == CLOUDS) {
            leds[fixed].g = period2;
          }
          if (weather == CLOUDS) {
            leds[fixed].r = period2;
          }
          break;
        case 3:
        case 9:
        case 13:
          if (weather == CLEAR || weather == CLOUDS) {
            leds[fixed].g = period3;
          }
          if (weather == CLOUDS) {
            leds[fixed].r = period3;
          }
          break;
        case 4:
        case 14:
        case 8:
          if (weather == CLEAR || weather == CLOUDS) {
            leds[fixed].g = period4;
          }
          if (weather == CLOUDS) {
            leds[fixed].r = period4;
          }
          break;
        case 6:
        case 16:
        case 11:
          if (weather == CLEAR || weather == CLOUDS) {
            leds[fixed].g = period5;
          }
          if (weather == CLOUDS) {
            leds[fixed].r = period5;
          }
          break;
      }
      if (weather == CLEAR) {
        leds[fixed].b = 0;
        leds[fixed].r = 255;
      }
      else if (weather == CLOUDS) {
        leds[fixed].b = 200;
      }
    }
  }
}

void raining() {
  for (int i = 0; i < NUM_LEDS; i++) {
    if (leds1[i].b == 255) {
      leds1[i].b = 15;
      if (i != 0) {
        leds1[i - 1].b = 255;
      }
    }
    else {
      leds1[i].r = 0;
      leds1[i].g = 0;
      leds1[i].b = 15;
    }
    if (leds1[i + 18].b == 255) {
      leds1[i + 18].b = 15;
      if (i != 0) {
        leds1[i + 17].b = 255;
      }
    }
    else {
      leds1[i + 18].r = 0;
      leds1[i + 18].g = 0;
      leds1[i + 18].b = 15;
    }
    if (leds1[i + 36].b == 255) {
      leds1[i + 36].b = 15;
      if (i != 0) {
        leds1[i + 35].b = 255;
      }
    }
    else {
      leds1[i + 36].r = 0;
      leds1[i + 36].g = 0;
      leds1[i + 36].b = 15;
    }
    if (leds1[i + 54].b == 255) {
      leds1[i + 54].b = 15;
      if (i != 0) {
        leds1[i + 53].b = 255;
      }
    }
    else {
      leds1[i + 54].r = 0;
      leds1[i + 54].g = 0;
      leds1[i + 54].b = 15;
    }
    if (leds1[i + 72].b == 255) {
      leds1[i + 72].b = 15;
      if (i != 0) {
        leds1[i + 71].b = 255;
      }
    }
    else {
      leds1[i + 72].r = 0;
      leds1[i + 72].g = 0;
      leds1[i + 72].b = 15;
    }
    if (leds1[i + 90].b == 255) {
      leds1[i + 90].b = 15;
      if (i != 0) {
        leds1[i + 89].b = 255;
      }
    }
    else {
      leds1[i + 90].r = 0;
      leds1[i + 90].g = 0;
      leds1[i + 90].b = 15;
    }
  }
  int randomizeTime = random8(100);
  if (randomizeTime > 90) {
    int randomize = random8(6);
    switch (randomize) {
      case 0:
        leds1[17].b = 255;
        break;
      case 1:
        leds1[35].b = 255;
        break;
      case 2:
        leds1[53].b = 255;
        break;
      case 3:
        leds1[71].b = 255;
        break;
      case 4:
        leds1[89].b = 255;
        break;
      case 5:
        leds1[107].b = 255;
        break;
    }
  }
}

void thunder() {
  int randomizeThunder = random8(6);
  for ( int i = 0; i < NUM_LEDS; i++) {
    switch (randomizeThunder) {
      case 0:
        leds1[i] = CRGB::White;
        break;
      case 1:
        leds1[i + 18] = CRGB::White;
        break;
      case 2:
        leds1[i + 36] = CRGB::White;
        break;
      case 3:
        leds1[i + 54] = CRGB::White;
        break;
      case 4:
        leds1[i + 72] = CRGB::White;
        break;
      case 5:
        leds1[i + 90] = CRGB::White;
        break;
    }
  }
  FastLED.delay(50);
  for ( int i = 0; i < NUM_LEDS; i++) {
    switch (randomizeThunder) {
      case 0:
        leds1[i] = CRGB::Black;
        leds1[i].b = 15;
        break;
      case 1:
        leds1[i + 18] = CRGB::Black;
        leds1[i + 18].b = 15;
        break;
      case 2:
        leds1[i + 36] = CRGB::Black;
        leds1[i + 36].b = 15;
        break;
      case 3:
        leds1[i + 54] = CRGB::Black;
        leds1[i + 54].b = 15;
        break;
      case 4:
        leds1[i + 72] = CRGB::Black;
        leds1[i + 72].b = 15;
        break;
      case 5:
        leds1[i + 90] = CRGB::Black;
        leds1[i + 90].b = 15;
        break;
    }
  }
}

void snowing() {
  for (int i = 0; i < NUM_LEDS; i++) {
    if (leds1[i] == CRGB(255, 255, 255)) {
      leds1[i] = CRGB(15, 15, 15);
      if (i != 0) {
        int mathm = random(1, 100);
        mathm = map(mathm, 1, 150, 1, 2);
        leds1[i - mathm] = CRGB(255, 255, 255);
      }
    }
    else {
      leds1[i] = CRGB(15, 15, 15);
    }
    if (leds1[i + 18] == CRGB(255, 255, 255)) {
      leds1[i + 18] = CRGB(15, 15, 15);
      if (i != 0) {
        int mathm = random(1, 100);
        mathm = map(mathm, 1, 150, 1, 2);
        mathm = mathm + 16;
        leds1[i  + mathm] = CRGB(255, 255, 255);
      }
    }
    else {
      leds1[i + 18] = CRGB(15, 15, 15);
    }
    if (leds1[i + 36] == CRGB(255, 255, 255)) {
      leds1[i + 36] = CRGB(15, 15, 15);
      if (i != 0) {
        int mathm = random(1, 100);
        mathm = map(mathm, 1, 130, 1, 2);
        mathm = mathm + 34;
        leds1[i  + mathm] = CRGB(255, 255, 255);
      }
    }
    else {
      leds1[i + 36] = CRGB(15, 15, 15);
    }
    if (leds1[i + 54] == CRGB(255, 255, 255)) {
      leds1[i + 54] = CRGB(15, 15, 15);
      if (i != 0) {
        int mathm = random(1, 100);
        mathm = map(mathm, 1, 150, 1, 2);
        mathm = mathm + 52;
        leds1[i + mathm] = CRGB(255, 255, 255);
      }
    }
    else {
      leds1[i + 54] = CRGB(15, 15, 15);
    }
    if (leds1[i + 72] == CRGB(255, 255, 255)) {
      leds1[i + 72] = CRGB(15, 15, 15);
      if (i != 0) {
        int mathm = random(1, 100);
        mathm = map(mathm, 1, 150, 1, 2);
        mathm = mathm + 70;
        leds1[i + mathm] = CRGB(255, 255, 255);
      }
    }
    else {
      leds1[i + 72] = CRGB(15, 15, 15);
    }
    if (leds1[i + 90] == CRGB(255, 255, 255)) {
      leds1[i + 90] = CRGB(15, 15, 15);
      if (i != 0) {
        int mathm = random(1, 100);
        mathm = map(mathm, 1, 140, 1, 2);
        mathm = mathm + 88;
        leds1[i  + mathm] = CRGB(255, 255, 255);
      }
    }
    else {
      leds1[i + 90] = CRGB(15, 15, 15);
    }
  }
  int randomizeTime = random8(100);
  if (randomizeTime > 70) {
    int randomize = random8(6);
    switch (randomize) {
      case 0:
        leds1[NUM_LEDS - 1] = CRGB(255, 255, 255);
        break;
      case 1:
        leds1[NUM_LEDS + 17] = CRGB(255, 255, 255);
        break;
      case 2:
        leds1[NUM_LEDS  + 35] = CRGB(255, 255, 255);
        break;
      case 3:
        leds1[NUM_LEDS + 53] = CRGB(255, 255, 255);
        break;
      case 4:
        leds1[NUM_LEDS + 71] = CRGB(255, 255, 255);
        break;
      case 5:
        leds1[NUM_LEDS + 89] = CRGB(255, 255, 255);
        break;
    }
  }
  FastLED.delay(random(160, 300));
}

void handleWeatherSettings() {                                                                // WEATHER SETTINGS PAGE
  if (serveur.hasArg("Latitude") && serveur.hasArg("Longitude") && serveur.hasArg("Forecast")) {
    latitude = serveur.arg("Latitude");
    longitude = serveur.arg("Longitude");
    forecast = serveur.arg("Forecast");
    preferences.putString("latitude", latitude);
    preferences.putString("longitude", longitude);
    preferences.putString("forecast", forecast);
    msg = "Sauvegargde r&eacute;ussie !";
    request_weather();
  }
  String content = "<meta http-equiv='content-type' content='text/html;charset=utf-8' />";
  content += "<meta name='viewport' content='width=device-width; initial-scale=1.0; maximum-scale=1.0;'>";
  content += "<html><body style='text-align:center'><form action='' method='POST'> R&eacute;glages M&eacute;t&eacute;o <br>";
  content += "Latitude:    <input type='text' id='Lat' name='Latitude' value='" + latitude + "' maxlength='11'><br>";
  content += "Longitude:<input type='text' id='Lng' name='Longitude' value='" + longitude + "' maxlength='11'><br>";
  content += "Pr&eacute;visions:  <select name=\"Forecast\">";
  content += "<option " + String((forecast == "0") ? "selected " : "") + "value=\"0\">Maintenant</option>";
  content += "<option " + String((forecast == "6") ? "selected " : "") + "value=\"6\">+6 heures</option>";
  content += "<option " + String((forecast == "12") ? "selected " : "") + "value=\"12\">+12 heures</option>";
  content += "<option " + String((forecast == "18") ? "selected " : "") + "value=\"18\">+18 heures</option>";
  content += "<option " + String((forecast == "24") ? "selected " : "") + "value=\"24\">+24 heures</option>";
  content += "<option " + String((forecast == "48") ? "selected " : "") + "value=\"48\">+2 jours</option>";
  content += "<option " + String((forecast == "72") ? "selected " : "") + "value=\"72\">+3 jours</option>";
  content += "<option " + String((forecast == "96") ? "selected " : "") + "value=\"96\">+4 jours</option>";
  content += "</select>";
  content += "<br><br><input type='submit' name='SUBMIT' value='Sauvegarder'></form>";
  content += "<a href='/'>Retour</a><br>";
  content += msg + "<br><br>";
  content += "<div id=\"googleMap\" style=\"width:100%;height:60%;text-align:center\"></div>";
  content += "<script> function myMap() {";
  content += "var mapProp= {center:new google.maps.LatLng(" + latitude + "," + longitude + "),zoom:11,};";
  content += "var map=new google.maps.Map(document.getElementById(\"googleMap\"),mapProp);";
  content += "google.maps.event.addListener(map, \"click\", function(event) {var lat = event.latLng.lat();var lng = event.latLng.lng();";
  content += "document.getElementById(\"Lat\").value=lat; document.getElementById(\"Lng\").value=lng;});}</script>";
  content += "<script src=\"https://maps.googleapis.com/maps/api/js?key=" + googleMapsID + "&callback=myMap\"></script>";
  content += "</body></html>";
  serveur.send(200, "text/html", content);
  msg = "";
}

void handleRoot() {                                                                               // MAIN PAGE
  //  if (!serveur.authenticate(username, string2char(pass))) {
  //    return serveur.requestAuthentication();
  //  }
  if (serveur.hasArg("COULEUR")) {
    String testteu = serveur.arg("COULEUR");
    if (testteu != 0) {
      derncoul = testteu;
      long number = strtol( &testteu[1], NULL, 16);
      color.r = number >> 16;
      color.g = number >> 8 & 0xFF;
      color.b = number & 0xFF;
      preferences.putString("derncoul", derncoul);
      if (color.r != rouge) {
        change_state("nw\n");
        change_state("r\n");
      }
      if (color.g != vert) {
        change_state("nw\n");
        change_state("g\n");
      }
      if (color.b != bleu) {
        change_state("nw\n");
        change_state("b\n");
      }
    }
  }
  if (serveur.hasArg("Effect")) {
    if (serveur.arg("Effect") == "Fire") {
      if (effect != FIRE) {
        change_state("f\n");
      }
      else {
        change_state("nf\n");
      }
    }
    if (serveur.arg("Effect") == "Weather") {
      if (effect != WEATHER) {
        change_state("w\n");
      }
      else {
        change_state("nw\n");
      }
    }
  }
  if (serveur.hasArg("Brightness")) {
    if (serveur.arg("Brightness") == "+") {
      if (brightness <= 75) {
        change_state("bu\n");
      }
      else {
        brightness = 100;
      }
    }
    else if (serveur.arg("Brightness") == "-") {
      if (brightness >= 25) {
        change_state("bd\n");
      } else {
        brightness = 0;
      }
    }
  }
  String content = "<meta http-equiv='content-type' content='text/html;charset=utf-8' />\n";
  content += "<meta charset=\"UTF-8\" />\n<title>Lampe Meteo</title>\n";
  content += "<meta name='viewport' content='width=device-width, initial-scale=1.0, maximum-scale=1.0'>\n";
  content += "<html><body style='text-align:center;'><form method='POST' action=''>\n";
  content += "<span style='font-size:30px'>Couleur</span><br>\n";
  content += "<input id=\"colorpad\" type=\"color\" name=\"COULEUR\" class=\"color\" value=\"" + derncoul + "\" style=\"height:50px; width:120px; margin:1px; opacity:";
  content += (effect != NO_EFFECT) ? "0.5;\" " : "1.0;\" ";
  content += "onchange=\"this.form.submit()\"" ">\n";
  content += "<br><br>\n";
  content += "<span style='font-size:30px'>Effets</span><br>\n";
  content += "<input type='Submit' name='Effect' value='Fire' style=\"height:50px; width:120px; margin:10px; font-size:0px; background-image:url('https://s3.amazonaws.com/spoonflower/public/design_thumbnails/0167/7306/rrFirePattern-01_shop_preview.png'); color:white; opacity:";
  content += (effect == FIRE) ? "1.0;\">" : "0.5;\">\n";
  content += "<input type='Submit' name='Effect' value='Weather' style=\"height:50px; width:120px; margin:10px; font-size:0px; background-image:url('http://images.all-free-download.com/images/graphiclarge/transparent_water_drops_design_background_vector_542481.jpg'); color:white; opacity:";
  content += (effect == WEATHER) ? "1.0;\">" : "0.5;\">\n";
  content += "<br>";
  content += "M&eacute;t&eacute;o : ";
  content += meteo;
  content += "<br>\n";
  content += "<span style='font-size:30px'>Intensit&eacute;e<br>\n";
  content += "<input type='Submit' name='Brightness' value='+' style='height:50px; width:120px; margin:10px; font-size:30px;'><br>\n";
  content += brightness;
  content += "%<br></span><input type='Submit' name='Brightness' value='-' style='height:50px; width:120px; margin:10px; font-size:30px;'>\n";
  content += "</form><br>\n";
  content += "<a href='/weather_settings'>R&eacute;glages m&eacute;t&eacute;o</a><br><br>\n";
  content += "</body></html>\n";
  serveur.send(200, "text/html", content);
}

void handleNotFound() {                             // NOT FOUND PAGE
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += serveur.uri();
  message += "\nMethod: ";
  message += (serveur.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += serveur.args();
  message += "\n";
  for (uint8_t i = 0; i < serveur.args(); i++) {
    message += " " + serveur.argName(i) + ": " + serveur.arg(i) + "\n";
  }
  serveur.send(404, "text/plain", message);
}

void request_weather() {
  int previousWeather = weather;
  int noOfBrackets;
  HTTPClient http;
  String url = "http://api.openweathermap.org/data/2.5/forecast?lat=" + latitude + "&lon=" + longitude + "&APPID=" + openWeatherID;
  http.begin(url);
  http.GET();
  if (forecast == "0") {
    noOfBrackets = 1;
  }
  else if (forecast == "6") {
    noOfBrackets = 3;
  }
  else if (forecast == "12") {
    noOfBrackets = 5;
  }
  else if (forecast == "18") {
    noOfBrackets = 7;
  }
  else if (forecast == "24") {
    noOfBrackets = 9;
  }
  else if (forecast == "48") {
    noOfBrackets = 17;
  }
  else if (forecast == "72") {
    noOfBrackets = 25;
  }
  else if (forecast == "96") {
    noOfBrackets = 33;
  }
  // parse json (forecast, 24h later)
  String payload = http.getString();
  int pos = payload.indexOf('[');
  if (pos == -1) {
    meteo = "erreur !";
    return;
  }
  for (int i = 0; i < noOfBrackets; i++) {
    pos = payload.indexOf('[', pos + 1 );
  }
  for (int i = 0; i < 5; i++) {
    pos = payload.indexOf("\"", pos + 1 );
  }
  String weatherString =  payload.substring(pos + 1, payload.indexOf("\"", pos + 2));
  if (weatherString == "Clear") {
    weather = CLEAR;
    meteo = "D&eacute;gag&eacute;";
    if (previousWeather != weather) {
      change_state("ws\n");
    }
  }
  else if (weatherString == "Clouds") {
    weather = CLOUDS;
    meteo = "Nuageux";
    if (previousWeather != weather) {
      change_state("wc\n");
    }
  }
  else if (weatherString == "Rain" || weatherString == "Drizzle") {
    weather = RAIN;
    meteo = "Pluie";
    if (previousWeather != weather) {
      change_state("wr\n");
    }
  }
  else if (weatherString == "Thunderstorm") {
    weather = THUNDERSTORM;
    meteo = "Orages";
    if (previousWeather != weather) {
      change_state("wt\n");
    }
  }
  else if (weatherString == "Snow") {
    weather = SNOW;
    meteo = "Neige";
    if (previousWeather != weather) {
      change_state("wx\n");
    }
  }
  http.end();
}

void loop1(void *pvParameters) {
  while (1) {
    serveur.handleClient();
    unsigned long now = millis();
    if (now > HOURS_TO_REQUEST_WEATHER * 3600000 && ((now - timerd) > HOURS_TO_REQUEST_WEATHER * 3600000 || (now - timerd) < 0)) {
      timerd = millis();
      request_weather();
    }
    vTaskDelay( 50 / portTICK_PERIOD_MS ); // wait / yield time to other tasks
  }
}

void setup() {
  //Serial.begin(115200);
  delay(300); // sanity delay
  FastLED.addLeds<CHIPSET, LED_PIN1, COLOR_ORDER>(leds1, NUM_LEDS2).setCorrection( CRGB( 185, 160, 100) );
  preferences.begin("meteo", false);
  latitude = preferences.getString("latitude", "45.49");
  longitude = preferences.getString("longitude", "-75.62");
  forecast = preferences.getString("forecast", "24");
  brightness = preferences.getUInt("BRIGHT_ADDR", 50);
  color.r = preferences.getUInt("RED_ADDR", 255);
  color.g = preferences.getUInt("GREEN_ADDR", 0);
  color.b = preferences.getUInt("BLUE_ADDR", 255);
  rouge = color.r;
  vert = color.g;
  bleu = color.b;
  effect = preferences.getUInt("EFFECT_ADDR", NO_EFFECT);
  weather = preferences.getUInt("WEATHER_ADDR", CLEAR);
  derncoul = preferences.getString("derncoul", "#FF00FF");
  change_state("setup");
  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback); //cree un callback pour savoir si passer par wifimanager
  wifiManager.setTimeout(240);
  WiFi.disconnect(); // pour prevenir de bugs de power et autres
  delay(500);
  if (!wifiManager.autoConnect("meteo")) {
    delay(3000);
    ESP.restart();
    delay(5000);
  }
  delay(500);
  if (manageur == 1) { // reboot si nouvelle config de wifimanager pour eviter bug de webserver qui roule pas
    ESP.restart();
    delay(5000);
  }
  MDNS.begin("meteo");
  serveur.on("/", handleRoot);
  serveur.on("/weather_settings", handleWeatherSettings);
  serveur.onNotFound(handleNotFound);
  serveur.begin();
  MDNS.addService("_http", "_tcp", 80);
  request_weather();
  xTaskCreatePinnedToCore(loop1, "loop1", 8192, NULL, 1, NULL, 0);
  delay(200);
}

void loop() {
    unsigned long unMillis = millis();
    if (unMillis - deuxMillis >= interval) {
      deuxMillis = unMillis;
      if (WiFi.status() != WL_CONNECTED) {
        WiFi.begin();
      }
    }
    if (effect == FIRE) {
      FRAMES_PER_SECOND = 80;
      Fire2012(leds1);
      FastLED.delay(random(900, 3000) / FRAMES_PER_SECOND);
      change = 1;
    }
    if (effect == WEATHER) {
      FRAMES_PER_SECOND = 50;
      int period0;
      int period1;
      int period2;
      int period3;
      int period4;
      int period5;
      if (weather == CLEAR || weather == CLOUDS) {
        if (weather == CLEAR) {
          period0 = beatsin16(3, 0, 145);
          period1 = beatsin16(11, 0, 145);
          period2 = beatsin16(17, 0, 145);
          period3 = beatsin16(23, 0, 145);
          period4 = beatsin16(31, 0, 145);
          period5 = beatsin16(49, 0, 145);
        }
        else if (weather == CLOUDS) {
          period0 = beatsin16(3, 0, 255);
          period1 = beatsin16(11, 0, 255);
          period2 = beatsin16(17, 0, 255);
          period3 = beatsin16(23, 0, 255);
          period4 = beatsin16(31, 0, 255);
          period5 = beatsin16(49, 0, 255);
        }
      }
      weather_effect(leds1, period0, period1, period2, period3, period4, period5);
      change = 1;
      if (weather == RAIN || weather == THUNDERSTORM) {
        FRAMES_PER_SECOND = 60;
        raining();
        if (weather == THUNDERSTORM) {
          int randomThunder = random8(100);
          if (randomThunder > 98) {
            thunder();
            change = 1;
          }
        }
      }
      else if (weather == SNOW) {
        FRAMES_PER_SECOND = 80;
        snowing();
        change = 1;
      }
    }
    if (runshow != 0) {
      runshow = 0;
      FastLED.setBrightness(map(brightness, 0, 100, 0, 210));
    }
    if (change != 0) {
      change = 0;
      FastLED.show();
    }
    FastLED.delay(1000 / FRAMES_PER_SECOND);
}
