// Wifi Lamp Project
// Steve Olmstead 10/02/2018
// Based on work from Vagtsal, 14/7/2017
#include <WiFi.h>
#include <ESPmDNS.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <FastLED.h>
#include <Preferences.h>
Preferences preferences;
#define HOURS_TO_REQUEST_WEATHER 1
String openWeatherID = "YourKeyHere";
String googleMapsID = "YourKeyHere";
unsigned long timerd = 0;
String username;
String pass;
String latitude;
String longitude;
String forecast;
String msg;
#define LED_PIN1     18
#define COLOR_ORDER RGB
#define CHIPSET     WS2812B
#define NUM_LEDS    18
#define NUM_LEDS2    108
#define COOLING  55
#define SPARKING 120
#define FRAMES_PER_SECOND 60
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
WebServer server(80);

// convert string 2 char*
char* string2char(String command) {
  if (command.length() != 0) {
    char *p = const_cast<char*>(command.c_str());
    return p;
  }
}

void change_state(String choice) {
  if (choice == "r\n") {
    color.r = 255;
    preferences.putUInt("RED_ADDR", 255);
  }
  else if (choice == "nr\n") {
    color.r = 0;
    preferences.putUInt("RED_ADDR", 0);
  }
  else if (choice == "g\n") {
    color.g = 255;
    preferences.putUInt("GREEN_ADDR", 255);
  }
  else if (choice == "ng\n") {
    color.g = 0;
    preferences.putUInt("GREEN_ADDR", 0);
  }
  else if (choice == "b\n") {
    color.b = 255;
    preferences.putUInt("BLUE_ADDR", 255);
  }
  else if (choice == "nb\n") {
    color.b = 0;
    preferences.putUInt("BLUE_ADDR", 0);
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
  int expBrightness = brightness * brightness;
  FastLED.setBrightness(map(expBrightness, 0, 10000, 0, 143));
  //   delay(1);
  FastLED.show();
  //   delay(1);
}

void Fire2012(CRGB leds[NUM_LEDS2]) {
  for (int imagod = 0; imagod < 6; imagod++) {
    // Array of temperature readings at each simulation cell
    static byte heat[NUM_LEDS];
    // Step 1.  Cool down every cell a little
    for ( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for ( int k = NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if ( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160, 255) );
    }
    // Step 4.  Map from heat cells to LED colors
    for ( int j = 0; j < NUM_LEDS; j++) {
      int multi = imagod * NUM_LEDS;
      int fixed = j + imagod;
      CRGB color = HeatColor( heat[fixed]);
      leds[fixed] = color;
    }
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
      int fixed = i * multi;
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
      if (i + 18 != 18) {
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
      if (i + 36 != 36) {
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
      if (i + 54 != 54) {
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
      if (i + 72 != 72) {
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
      if (i + 90 != 90) {
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
        leds1[18 - 1].b = 255;
        break;
      case 1:
        leds1[36 - 1].b = 255;
        break;
      case 2:
        leds1[54 - 1].b = 255;
        break;
      case 3:
        leds1[72 - 1].b = 255;
        break;
      case 4:
        leds1[90 - 1].b = 255;
        break;
      case 5:
        leds1[108 - 1].b = 255;
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
        leds1[i - 1] = CRGB(255, 255, 255);
      }
    }
    else {
      leds1[i] = CRGB(15, 15, 15);
    }
    if (leds1[i + 18] == CRGB(255, 255, 255)) {
      leds1[i + 18] = CRGB(15, 15, 15);
      if (i != 0) {
        leds1[i  + 17] = CRGB(255, 255, 255);
      }
    }
    else {
      leds1[i + 18] = CRGB(15, 15, 15);
    }
    if (leds1[i + 36] == CRGB(255, 255, 255)) {
      leds1[i + 36] = CRGB(15, 15, 15);
      if (i != 0) {
        leds1[i  + 35] = CRGB(255, 255, 255);
      }
    }
    else {
      leds1[i + 36] = CRGB(15, 15, 15);
    }
    if (leds1[i + 54] == CRGB(255, 255, 255)) {
      leds1[i + 54] = CRGB(15, 15, 15);
      if (i != 0) {
        leds1[i + 53] = CRGB(255, 255, 255);
      }
    }
    else {
      leds1[i + 54] = CRGB(15, 15, 15);
    }
    if (leds1[i + 72] == CRGB(255, 255, 255)) {
      leds1[i + 72] = CRGB(15, 15, 15);
      if (i != 0) {
        leds1[i + 71] = CRGB(255, 255, 255);
      }
    }
    else {
      leds1[i + 72] = CRGB(15, 15, 15);
    }
    if (leds1[i + 90] == CRGB(255, 255, 255)) {
      leds1[i + 90] = CRGB(15, 15, 15);
      if (i != 0) {
        leds1[i  + 89] = CRGB(255, 255, 255);
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
  FastLED.delay(300);
}

void handleAdminSettings() {                                                                // ADMIN SETTINGS PAGE
  if (!server.authenticate(string2char(username), string2char(pass))) {
    return server.requestAuthentication();
  }
  if (server.hasArg("USERNAME") && server.hasArg("PASS")) {
    username = server.arg("USERNAME");
    pass = server.arg("PASS");
    preferences.putString("username", username);
    preferences.putString("pass", pass);
    msg = "Save Successfull!";
  }
  String content = "<meta http-equiv='content-type' content='text/html;charset=utf-8' />";
  content += "<meta name='viewport' content='width=device-width; initial-scale=1.0; maximum-scale=1.0;'>";
  content += "<html><body style='text-align:center'><form action='' method='POST'> Admin Settings:<br>";
  content += "User name:<input type='text' name='USERNAME' value='" + username + "' maxlength='20'><br>";
  content += "Password:<input type='password' name='PASS' value='" + pass + "' maxlength='20'><br><br>";
  content += "<input type='submit' name='SUBMIT' value='Save'></form><br>";
  content += msg + "<br><br>";
  content += "<a href='/'>Back</a></body></html>";
  server.send(200, "text/html", content);
  msg = "";
}

void handleWeatherSettings() {                                                                // WEATHER SETTINGS PAGE
  if (!server.authenticate(string2char(username), string2char(pass))) {
    return server.requestAuthentication();
  }
  if (server.hasArg("Latitude") && server.hasArg("Longitude") && server.hasArg("Forecast")) {
    latitude = server.arg("Latitude");
    longitude = server.arg("Longitude");
    forecast = server.arg("Forecast");
    preferences.putString("latitude", latitude);
    preferences.putString("longitude", longitude);
    preferences.putString("forecast", forecast);
    msg = "Save Successfull!";
    request_weather();
  }
  String content = "<meta http-equiv='content-type' content='text/html;charset=utf-8' />";
  content += "<meta name='viewport' content='width=device-width; initial-scale=1.0; maximum-scale=1.0;'>";
  content += "<html><body style='text-align:center'><form action='' method='POST'> Weather Settings <br>";
  content += "Latitude:    <input type='text' id='Lat' name='Latitude' value='" + latitude + "' maxlength='11'><br>";
  content += "Longitude:<input type='text' id='Lng' name='Longitude' value='" + longitude + "' maxlength='11'><br>";
  content += "Forecast:  <select name=\"Forecast\">";
  content += "<option " + String((forecast == "0") ? "selected " : "") + "value=\"0\">Now</option>";
  content += "<option " + String((forecast == "6") ? "selected " : "") + "value=\"6\">+6  hours</option>";
  content += "<option " + String((forecast == "12") ? "selected " : "") + "value=\"12\">+12 hours</option>";
  content += "<option " + String((forecast == "18") ? "selected " : "") + "value=\"18\">+18 hours</option>";
  content += "<option " + String((forecast == "24") ? "selected " : "") + "value=\"24\">+24 hours</option>";
  content += "<option " + String((forecast == "48") ? "selected " : "") + "value=\"48\">+2  days</option>";
  content += "<option " + String((forecast == "72") ? "selected " : "") + "value=\"72\">+3  days</option>";
  content += "<option " + String((forecast == "96") ? "selected " : "") + "value=\"96\">+4  days</option>";
  content += "</select>";
  content += "<br><br><input type='submit' name='SUBMIT' value='Save'></form>";
  content += "<a href='/'>Back</a><br>";
  content += msg + "<br><br>";
  content += "<div id=\"googleMap\" style=\"width:100%;height:60%;text-align:center\"></div>";
  content += "<script> function myMap() {";
  content += "var mapProp= {center:new google.maps.LatLng(" + latitude + "," + longitude + "),zoom:11,};";
  content += "var map=new google.maps.Map(document.getElementById(\"googleMap\"),mapProp);";
  content += "google.maps.event.addListener(map, \"click\", function(event) {var lat = event.latLng.lat();var lng = event.latLng.lng();";
  content += "document.getElementById(\"Lat\").value=lat; document.getElementById(\"Lng\").value=lng;});}</script>";
  content += "<script src=\"https://maps.googleapis.com/maps/api/js?key=" + googleMapsID + "&callback=myMap\"></script>";
  content += "</body></html>";
  server.send(200, "text/html", content);
  msg = "";
}

void handleRoot() {                                                                               // MAIN PAGE
  //  if (!server.authenticate(username, string2char(pass))) {
  //    return server.requestAuthentication();
  //  }
  if (server.hasArg("Color")) {
    if (server.arg("Color") == "Red") {
      if (color.r == 0) {
        change_state("r\n");
      }
      else {
        change_state("nr\n");
      }
    }
    else if (server.arg("Color") == "Green") {
      if (color.g == 0) {
        change_state("g\n");
      }
      else {
        change_state("ng\n");
      }
    }
    else if (server.arg("Color") == "Blue") {
      if (color.b == 0) {
        change_state("b\n");
      }
      else {
        change_state("nb\n");
      }
    }
  }
  if (server.hasArg("Effect")) {
    if (server.arg("Effect") == "Fire") {
      if (effect != FIRE) {
        change_state("f\n");
      }
      else {
        change_state("nf\n");
      }
    }
    if (server.arg("Effect") == "Weather") {
      if (effect != WEATHER) {
        change_state("w\n");
      }
      else {
        change_state("nw\n");
      }
    }
  }
  if (server.hasArg("Brightness")) {
    if (server.arg("Brightness") == "+") {
      if (brightness <= 75) {
        change_state("bu\n");
      }
      else {
        brightness = 100;
      }
    }
    else if (server.arg("Brightness") == "-") {
      if (brightness >= 25) {
        change_state("bd\n");
      } else {
        brightness = 0;
      }
    }
  }
  String content = "<meta http-equiv='content-type' content='text/html;charset=utf-8' />";
  content += "<meta name='viewport' content='width=device-width; initial-scale=1.0; maximum-scale=1.0;'>";
  content += "<html><body style='text-align:center;'><form method='POST' action=''>";
  content += "<span style='font-size:30px'>Colors</span><br>";
  content += "<input type='Submit' name='Color' value='Red' style='height:50px; width:120px; margin:10px; font-size:0px; background-color:red; color:white; opacity:";
  content += (color.r == 0) ? "0.5;'" : "1.0;'";
  content += (effect != NO_EFFECT) ? " disabled>" : ">";
  content += "<input type='Submit' name='Color' value='Green' style='height:50px; width:120px; margin:10px; font-size:0px; background-color:green; color:white; opacity:";
  content += (color.g == 0) ? "0.5;'" : "1.0;'";
  content += (effect != NO_EFFECT) ? " disabled>" : ">";
  content += "<br>";
  content += "<input type='Submit' name='Color' value='Blue' style='height:50px; width:120px; margin:10px; font-size:0px; background-color:blue; color:white; opacity:";
  content += (color.b == 0) ? "0.5;'" : "1.0;'";
  content += (effect != NO_EFFECT) ? " disabled>" : ">";
  content += "<br><br>";
  content += "<span style='font-size:30px'>Effects</span><br>";
  content += "<input type='Submit' name='Effect' value='Fire' style=\"height:50px; width:120px; margin:10px; font-size:0px; background-image:url('https://s3.amazonaws.com/spoonflower/public/design_thumbnails/0167/7306/rrFirePattern-01_shop_preview.png'); color:white; opacity:";
  content += (effect == FIRE) ? "1.0;\">" : "0.5;\">";
  content += "<input type='Submit' name='Effect' value='Weather' style=\"height:50px; width:120px; margin:10px; font-size:0px; background-image:url('http://images.all-free-download.com/images/graphiclarge/transparent_water_drops_design_background_vector_542481.jpg'); color:white; opacity:";
  content += (effect == WEATHER) ? "1.0;\">" : "0.5;\">";
  content += "<br><br>";
  content += "<span style='font-size:30px'>Brightness<br>";
  content += "<input type='Submit' name='Brightness' value='+' style='height:50px; width:120px; margin:10px; font-size:30px;'><br>";
  content += brightness;
  content += "%<br></span><input type='Submit' name='Brightness' value='-' style='height:50px; width:120px; margin:10px; font-size:30px;'>";
  content += "</form><br>";
  content += "<a href='/admin_settings'>Admin Settings</a><br>";
  content += "<a href='/weather_settings'>Weather Settings</a><br><br>";
  content += "</body></html>";
  server.send(200, "text/html", content);
}

void handleNotFound() {                             // NOT FOUND PAGE
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void request_weather() {
  int previousWeather = weather;
  int noOfBrackets;
  Serial.println("depart");
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
  //Serial.println(payload);
  int pos = payload.indexOf('[');
  if (pos == -1) {
    Serial.println("error");
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
    change_state("ws\n");
  }
  else if (weatherString == "Clouds") {
    weather = CLOUDS;
    change_state("wc\n");
  }
  else if (weatherString == "Rain" || weatherString == "Drizzle") {
    weather = RAIN;
    change_state("wr\n");
  }
  else if (weatherString == "Thunderstorm") {
    weather = THUNDERSTORM;
    change_state("wt\n");
  }
  else if (weatherString == "Snow") {
    weather = SNOW;
    change_state("wx\n");
  }
  if (previousWeather != weather) {
    Serial.println("different");
  }
  http.end();
  Serial.println("fini");
}

void loop1(void *pvParameters) {
  while (1) {
    server.handleClient();
    unsigned long now = millis();
    if (now > HOURS_TO_REQUEST_WEATHER * 3600000 && ((now - timerd) > HOURS_TO_REQUEST_WEATHER * 3600000 || (now - timerd) < 0)) {
      timerd = millis();
      request_weather();
    }
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait / yield time to other tasks
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000); // sanity delay
  FastLED.addLeds<CHIPSET, LED_PIN1, COLOR_ORDER>(leds1, NUM_LEDS2).setCorrection( TypicalLEDStrip );
  preferences.begin("meteo", false);
  username = preferences.getString("username", "admin");
  pass = preferences.getString("pass", "admin");
  latitude = preferences.getString("latitude", "50");
  longitude = preferences.getString("longitude", "50");
  forecast = preferences.getString("forecast", "24");
  brightness = preferences.getUInt("BRIGHT_ADDR", 50);
  color.r = preferences.getUInt("RED_ADDR", 255);
  color.g = preferences.getUInt("GREEN_ADDR", 255);
  color.b = preferences.getUInt("BLUE_ADDR", 255);
  effect = preferences.getUInt("EFFECT_ADDR", NO_EFFECT);
  weather = preferences.getUInt("WEATHER_ADDR", CLEAR);
  //change_state("nw\n");
  WiFiManager wifiManager;
  wifiManager.setTimeout(240);
  WiFi.disconnect(); // pour prevenir de bugs de power et autres
  delay(1000);
  if (!wifiManager.autoConnect()) {
    delay(3000);
    ESP.restart();
    delay(5000);
  }
  MDNS.begin("meteo");
  server.on("/", handleRoot);
  server.on("/weather_settings", handleWeatherSettings);
  server.on("/admin_settings", handleAdminSettings);
  server.onNotFound(handleNotFound);
  const char * headerkeys[] = {"User-Agent", "Referer"} ;//peut ajouter autres si besoin eg {"User-Agent", "Cookie"}
  size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
  server.collectHeaders(headerkeys, headerkeyssize );
  server.begin();
  MDNS.addService("_http", "_tcp", 80);
  //Serial.print("Server parti");
  request_weather();
  xTaskCreatePinnedToCore(loop1, "loop1", 4096, NULL, 2, NULL, 0);
}

void loop() {
  if (effect == FIRE) {
    Fire2012(leds1);
  }
  if (effect == WEATHER) {
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
    if (weather == RAIN || weather == THUNDERSTORM) {
      raining();
      if (weather == THUNDERSTORM) {
        int randomThunder = random8(100);
        if (randomThunder > 98) {
          thunder();
        }
      }
      FastLED.delay(1000 / 60);
    }
    else if (weather == SNOW) {
      snowing();
    }
  }
  //delay(1);
  FastLED.show();
  //delay(1);
  FastLED.delay(1000 / FRAMES_PER_SECOND);
  //  vTaskDelay( 1000 / FRAMES_PER_SECOND );
}
