
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

int sensorPin = A0;    // select the input pin for the potentiometer
int lightPin = D2;

int heatingPin = D5;


ESP8266WiFiMulti WiFiMulti;
// WiFiServer server(443);
char *host = "poultry.zeusakalam.com";


float value=30;
bool isLight=false;

void setup() {
    Serial.begin(115200);     // Starts the serial communication
    Serial.println(system_get_chip_id());
    pinMode(lightPin,OUTPUT);
    pinMode(heatingPin,OUTPUT);
    WiFi.mode(WIFI_STA);
    // WiFiMulti.addAP("money-changer", "c0in1234");
    
//   WiFiMulti.addAP("C12", "epoultry");
    WiFiMulti.addAP("PLDTHOMEFIBRWFHX5", "@Tanongmokaymama086");
    waitNetwork();
    checkconfig();
}


void loop() {
  float sensorValue = analogRead(sensorPin);
  Serial.println(sensorValue);
  if(sensorValue<=value)
  {
    digitalWrite(lightPin,HIGH);
    if(!isLight){
        post("/api/setlight","state=on");
        isLight=true;
    }
  }
  else{
    digitalWrite(lightPin,LOW);
    if(isLight){
        post("/api/setlight","state=off");
        // post("/api/setwaterlog","state=off");
        isLight=false;
    }
  }
  
//   String payload=request("/api/getheatstatus");

//      // Use https://arduinojson.org/v6/assistant to compute the capacity.
//     StaticJsonDocument<96> doc;

//     DeserializationError error = deserializeJson(doc, payload);

//     if (error)
//     {
//       Serial.print(F("deserializeJson() failed: "));
//       Serial.println(error.f_str());
//       return;
//     }
//     int val = doc["heating"];
//     digitalWrite(heatingPin,val==1?HIGH:LOW);
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
void checkconfig(){
    String payload=request("/api/getlightconf");

     // Use https://arduinojson.org/v6/assistant to compute the capacity.
    StaticJsonDocument<96> doc;

    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    value = doc["value"];
}
