#include <Arduino.h>

#include <ESPmDNS.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define DEBUG 1

#define PUSH_INTERVAL 10 // in seconds

// Change these lines to match your environment
#define SSID     "enter your ssid here"
#define PASSWORD "enter your ssid password here"
const char* http_endpoint = "http://<myhttpendpoint>";

struct spindel {
    // static
    int index;
    String name;
    String token;
    // dynamic
    bool enabled;
    uint16_t angle;       // 
    uint16_t gravity;     // SG * 1000 
    uint16_t temperature; // Deg F 
};

spindel spindel_array[] = {
  { 0, "iSpindel1", "token", true, 25, 1000, 42 },
  { 1, "iSpindel2", "token", true, 25, 1000, 42 },
  { 2, "iSpindel3", "token", true, 25, 1000, 42 },
  { 3, "iSpindel4", "token", true, 25, 1000, 42 },
  { 4, "iSpindel5", "token", true, 25, 1000, 42 },
};

AsyncWebServer server(80);

void setup() {
#if DEBUG
  Serial.begin(115200);
#endif

  if (WiFi.begin( SSID , PASSWORD ) == WL_CONNECT_FAILED) {
#if DEBUG
     Serial.println("Error starting WiFi");
#endif
  } else {
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
#if DEBUG
      Serial.print(".");
#endif
    }

    if (!MDNS.begin("ispindel-sim")) {
  #if DEBUG
      Serial.println("Error starting mDNS");
  #endif
    } else {
      MDNS.addService("http", "tcp", 80);
    }

    Serial.println( WiFi.localIP().toString() );

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {    
      String html = "<html><head><title>iSpindel Simulator</title>";
      String refresh = request->arg("refresh");
      if (refresh.length() > 0) {
        html += "<meta http-equiv=\"refresh\" content=\"" + refresh + "\">";
      }
      html += "</head><body><h1>iSpindel Simulator</h1>";
      html += "<table>";
      html += "<tr><td>Name</td><td>Active</td><td>Gravity</td><td>Temperature</td></tr>";
      for(auto spindel: spindel_array) {
        html += "<tr><td>" + spindel.name + "</td><td align='center'>";
        if (spindel.enabled)
          html += "on";
        else
          html += "off";
        float g = spindel.gravity / 1000.0f;
        html += "</td><td align='right'>" + String(g, 3) + " sg</td>";
        float t = spindel.temperature / 1.0f;
        html += "<td align='right'>" + String(t, 0) + "&deg;F</td></tr>";
      }
      html += "</table>";
      html += "<p>A simple REST API can be use to control the simulated devices. Examples:</p>";
      html += "<p>Disable all devices:</p>";
      String ip = WiFi.localIP ().toString ();
      html += "<code>curl \"http://" + ip + "/set?name=*&active=off\"</code>";
      html += "<p>Enable the <b>Name</b> device with specific gravity and temperature values:</p>";
      html += "<code>curl \"http://" + ip + "/set?name=ispindel001&active=on&angle=25&sg=1.2001&temp=65.1\"</code>";
      html += "</body></html>";
      request->send(200, "text/html", html);
    });

    server.on("/set", HTTP_GET, [](AsyncWebServerRequest *request) {
      // name is required, everything else is optional
     int status = 404;
      String name = request->arg("name");
      for(auto s: spindel_array) {
        if ((name == "*") || (s.name == name)) {
          status = 200;
          String active = request->arg("active");
          if (active == "on")
            s.enabled = true;
          else if (active == "off")
            s.enabled = false;

          String angle = request->arg("angle");
          if (angle.length() > 0)
            s.angle = (uint16_t) angle.toDouble() ;

          String sg = request->arg("sg");
          if (sg.length() > 0)
            s.gravity = (uint16_t) (sg.toDouble() * 1000);

          String temp = request->arg("temp");
          if (temp.length() > 0)
            s.temperature = (uint16_t) (temp.toDouble() * 1);

#if DEBUG
          Serial.printf ("%s %s %d %F sg %d F\n", s.name.c_str (), s.enabled ? "on" : "off", s.angle, s.gravity / 1000.0f, s.temperature);
#endif
          spindel_array[s.index] = s;
        }
      }
      request->send(status, "text/html");
    });

    server.begin();
  }
}

void setBeacon(spindel s) {

    StaticJsonDocument<400> doc;

    // For testing purposes we use the iSpindel endpoint at fermentrack
    doc["name"]        = s.name;
    doc["ID"]          = "MYID";
    doc["token"]       = s.token; 
    doc["angle"]       = s.angle; 
    doc["temperature"] = s.temperature; 
    doc["temp_units"]  = "F"; 
    doc["battery"]     = 4.2; 
    doc["gravity"]     = s.gravity; 
    doc["interval"]    = 300; 
    doc["RSSI"]        = WiFi.RSSI(); 

    WiFiClient client;
    HTTPClient http;
    String serverPath = http_endpoint;

    // Your Domain name with URL path or IP address with path
    http.begin( client, serverPath);
    String json;
    serializeJson(doc, json);
#if DEBUG
    serializeJson(doc, Serial);
    Serial.println();
#endif

    if (http.POST(json)!=200) 
      Serial.println("Failed to send json");
 
    http.end();
}

void loop() {
  for(auto spindel: spindel_array) {
    if (!spindel.enabled)
      continue;
    setBeacon(spindel);
    delay( PUSH_INTERVAL );
  }
}
