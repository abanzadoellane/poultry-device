/*
 * Created by Pi BOTS MakerHub
 *
 * Email: pibotsmakerhub@gmail.com
 * 
 * Github: https://github.com/pibotsmakerhub
 *
 * Join Us on Telegram : https://t.me/pibots 
 * Copyright (c) 2020 Pi BOTS MakerHub
*/

// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor

#include "DHT.h"
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#define DHTPIN D6     // Digital pin connected to the DHT sensor
#define FANPIN D5     // Digital pin connected to the DHT sensor

// #define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.

ESP8266WiFiMulti WiFiMulti;
// WiFiServer server(443);
char *host = "poultry.zeusakalam.com";

DHT dht(DHTPIN, DHTTYPE);

long interval=10000;

void setup() {
    Serial.begin(115200);     // Starts the serial communication
    Serial.println(system_get_chip_id());
    pinMode(FANPIN,OUTPUT);
    WiFi.mode(WIFI_STA);
    // WiFiMulti.addAP("money-changer", "c0in1234");
//   WiFiMulti.addAP("C12", "june152001");
    WiFiMulti.addAP("PLDTHOMEFIBRWFHX5", "@Tanongmokaymama086");
    dht.begin();
    waitNetwork();
    senddata();
}


void loop() {
  senddata();
  delay(interval);
}
String request(String path)
{
    WiFiClientSecure client;
    // WiFiClient client;
    Serial.print("connecting to ");
    Serial.println(host);
    client.setInsecure();

    if (!client.connect(host, 443))
    {
        Serial.println("connection failed");
        return "error";
    }
    Serial.print("requesting URL: ");
    Serial.print(host);
    Serial.println(path);

    String q = String("GET ") + path + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + "\r\n\r\n"; // termination
    Serial.println(q);
    client.print(q);
    Serial.println("request sent");
    while (client.connected())
    {
        String line = client.readStringUntil('\n');
        if (line == "\r")
        {
            Serial.println("headers received");
            break;
        }
    }
    String line = client.readStringUntil('\n');
    Serial.println("reply was:");
    Serial.println("==========");
    Serial.println(line);
    Serial.println("==========");
    Serial.println("closing connection");
    return line;
}
void waitNetwork()
{   
    Serial.println("waiting for wifi connection");
    while ((WiFiMulti.run() != WL_CONNECTED))
    {
        Serial.print(".");
        delay(300);
    }
}
String post(String path,String message)
{
    WiFiClientSecure client;
    // WiFiClient client;
    Serial.print("connecting to ");
    Serial.println(host);
    client.setInsecure();

    if (!client.connect(host, 443))
    {
        Serial.println("connection failed");
        return "error";
    }
    Serial.print("requesting URL: ");
    Serial.print(host);
    Serial.println(path);

    String q = String("POST ") + path + " HTTP/1.1\r\n" +
    "Content-Type: application/x-www-form-urlencoded\r\n" +
               "Host: " + host + "\r\n"+
               "Accept: application/json"+"\r\n"+
               "Content-Length: "+String(message.length())+"\r\n"+ // termination
               "\r\n" + message
           + "\r\n";
    Serial.println(q);
    client.print(q);
    Serial.println("request sent");
    while (client.connected())
    {
        String line = client.readStringUntil('\n');
        if (line == "\r")
        {
            Serial.println("headers received");
            break;
        }
    }
    String line = client.readStringUntil('\n');
    Serial.println("reply was:");
    Serial.println("==========");
    Serial.println(line);
    Serial.println("==========");
    Serial.println("closing connection");
    return line;
}
void senddata(){
    String payload=request("/api/getdhtconf");

     // Use https://arduinojson.org/v6/assistant to compute the capacity.
    StaticJsonDocument<96> doc;

    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    interval = doc["countdown"];

    float hcrit=doc["hcrit"];
    float tcrit=doc["tcrit"];
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht.readTemperature(true);

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)) {
        Serial.println(F("Failed to read from DHT sensor!"));
        return;
    }

    // Compute heat index in Fahrenheit (the default)
    float hif = dht.computeHeatIndex(f, h);
    // Compute heat index in Celsius (isFahreheit = false)
    float hic = dht.computeHeatIndex(t, h, false);

    Serial.print(F(" Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F("C "));
    Serial.print(f);
    Serial.print(F("F  Heat index: "));
    Serial.print(hic);
    Serial.print(F("C "));
    Serial.print(hif);
    Serial.println(F("F"));
    post("/api/sethdt","humidity="+String(h)+"&temperature="+String(t));
    if(t>tcrit)
    {
        post("/api/setfan","state=on");
        digitalWrite(FANPIN,1);
    }
    else
    {
        post("/api/setfan","state=off");
        digitalWrite(FANPIN,0);
    }
}
