/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp8266-nodemcu-hc-sr04-ultrasonic-arduino/

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

ESP8266WiFiMulti WiFiMulti;

// Set web server port number to 80
// WiFiServer server(443);
WiFiServer server(80);

char *host = "poultry.zeusakalam.com";
// char *host = "192.168.95.152";

const int trigPin = D5;
const int echoPin = D6;
const int pumppin = D2;
const int trig2 = D3;
const int echo2 = D4;

// define sound velocity in cm/uS
#define SOUND_VELOCITY 0.034
#define CM_TO_INCH 0.393701

bool watering=false;
float critical=15;
float full=2;

void setup()
{
    Serial.begin(115200);     // Starts the serial communication
    Serial.println(system_get_chip_id());
    pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
    pinMode(echoPin, INPUT);  // Sets the echoPin as an Input
    pinMode(pumppin, OUTPUT);

    pinMode(trig2, OUTPUT); // Sets the trigPin as an Output
    pinMode(echo2, INPUT);  // Sets the echoPin as an Input

    WiFi.mode(WIFI_STA);
    // WiFiMulti.addAP("ITRec", "ITRec123!");
    WiFiMulti.addAP("money-changer", "c0in1234");
    // WiFiMulti.addAP("C12", "epoultry");
    // WiFiMulti.addAP("PLDTHOMEFIBRWFHX5", "@Tanongmokaymama086");
    waitNetwork();
    senddata();
}

void loop()
{
    Serial.print("dispenser");
    float val=sense(trigPin, echoPin);
    Serial.print("main");
    float val2=sense(trig2, echo2);
    // Serial.print("val ");
    // Serial.println(val);
    // Serial.print("crit ");
    // Serial.println(critical);
    // Serial.println(watering);
    if ( val>= critical)
    {
        Serial.println("dispensing");
        digitalWrite(pumppin, 1);
        if(!watering){
            post("/api/setwaterlog","tank=dispenser");
            watering=true;
        }
    }
    else if(val<=full)
    {
        Serial.println("tank was full");
        digitalWrite(pumppin, 0);
        if(watering){
            post("/api/settanklevel","tank=main&level="+String(val2));
            // post("/api/setwaterlog","state=off");
            watering=false;
        }
    }
    delay(500);
}

float sense(int trig, int echo)
{

    long duration;
    float distanceCm;
    float distanceInch;
    // Clears the trigPin
    digitalWrite(trig, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);

    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(echo, HIGH);

    // Calculate the distance
    distanceCm = duration * SOUND_VELOCITY / 2;

    // Convert to inches
    distanceInch = distanceCm * CM_TO_INCH;

    // Prints the distance on the Serial Monitor
    // Serial.print("Distance (cm): ");
    // Serial.println(distanceCm);
    Serial.print("Distance (inch): ");
    Serial.println(distanceInch);
    return distanceInch;
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
    String payload=request("/api/checkmode");

     // Use https://arduinojson.org/v6/assistant to compute the capacity.
    StaticJsonDocument<96> doc;

    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    String mode = doc["mode"];
    critical=doc["critical"];
    full=doc["fill"];
    if(mode=="setup")
    {
        Serial.println(mode);
        float tank1=sense(trigPin, echoPin);
        float tank2=sense(trig2, echo2);
        post("/api/tank/setheight","maintankheight="+String(tank2)+"&dispensertankheight="+String(tank1));
    }
}