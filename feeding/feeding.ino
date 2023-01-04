#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <Servo.h>

ESP8266WiFiMulti WiFiMulti;
#define ServoPin D5   //D5 is GPIO14
Servo myservo;  // create servo object to control a servo

// define sound velocity in cm/uS
#define SOUND_VELOCITY 0.034
#define CM_TO_INCH 0.393701

const int trigPin = D5;
const int echoPin = D6;

// Set web server port number to 80
WiFiServer server(443);

char *host = "poultry.zeusakalam.com";
char *path = "/api/check";

float critical=0;
float height=0;

void setup()
{
  myservo.attach(ServoPin); // attaches the servo on GIO2 to the servo object

  Serial.begin(115200);
    pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
    pinMode(echoPin, INPUT);  // Sets the echoPin as an Input
  // Serial.setDebugOutput(true);
  // pinMode(BUILTIN_LED, OUTPUT);

  WiFi.mode(WIFI_STA);
  // WiFiMulti.addAP("ITRec", "ITRec123!");
  WiFiMulti.addAP("C12", "june152001");
  WiFiMulti.addAP("money-changer", "c0in1234");
}

void loop()
{
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED))
  {
    String payload=request(path);

    // Use https://arduinojson.org/v6/assistant to compute the capacity.
    StaticJsonDocument<96> doc;

    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    // const char *date = doc["date"]; // "2022-12-06"
    // const char *time = doc["time"]; // "15:38:19"
    int feeding = doc["feeding"]; // 0
    // Serial.println(date);
    // Serial.println(time);
    Serial.println(feeding);
    // digitalWrite(BUILTIN_LED, feeding);
    if(feeding==0)
      myservo.write(0);
    else
      myservo.write(180);
  }
  float val=sense(trigPin, echoPin);


  delay(500);
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
void getConf()
{
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
    critical=doc["crit"];
    height=doc["height"];
    if(mode=="setup")
    {
        Serial.println(mode);
        float tank=sense(trigPin, echoPin);
        post("/api/feeder/tank","height="+String(tank));
    }
}
