#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Servo.h>  

//***WiFi network SSID and Password and MQTT broker address***//
const char* ssid = "ShipperBeeDemo";
const char* password = "Danby123";

const char* subTopic="mailbox/doorControl";
const char* mqtt_server = "broker.mqtt-dashboard.com";
const char* deviceID="ShipperBee-";


//***Global Variables***//
WiFiClient espClient;
PubSubClient client(espClient);

//***Servo library***//
Servo myservo;  //create servo object to control a servo

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  /*Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);*/

  //***WiFi connection manager method (Blocking Function)***//
  WiFiManager wifiManager;
  wifiManager.autoConnect(ssid, password);
  //wifiManager.autoConnect("ShipperBeeDemo", "Danby123");
 
  //***Method for connecting to WiFi with hardcoded credentials***//
  //WiFi.begin(ssid, password);

  //***Check to see WiFi Connection Status***//
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //***Seed for RNG for unique client ID***//
  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    //String clientId = "ShipperBee-";
    String clientId = deviceID;
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // resubscribe
      //client.subscribe("mailbox/doorControl");
      client.subscribe(subTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//***Callback for parsing messages on subscribed topic***//
void callback(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<100> jsonBuffer; //Set Json document buffer size
  char tempbu[length+1];

  //***Parse Json document received from subscribed topic***//
  if(strcmp(topic,"mailbox/doorControl")==0){
    snprintf(tempbu, length+1, "%s", payload);
    DeserializationError error = deserializeJson(jsonBuffer, tempbu);

  //***Check to see any Json parsing errors (Debug only)
    /*if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }*/
    //***Load Json document into root and extract result from the Status variable***//
    JsonObject root = jsonBuffer.as<JsonObject>();
  
    const char* outbd=root["Status"];
    Serial.println(outbd);

    //***Parse response and execute command***//
    if(strcmp(outbd,"Open")==0){
     /* digitalWrite(BUILTIN_LED, LOW);
      digitalWrite(12, HIGH);
      delay(1000);
      digitalWrite(12,LOW);
      digitalWrite(BUILTIN_LED,HIGH);*/
      
      myservo.write(140); //Servo open door positon
      delay(300);
      myservo.write(100); //Servo return.
    } else{
      digitalWrite(BUILTIN_LED, HIGH);
    }
  }
  memcpy(tempbu,"\0",sizeof(tempbu)); //Clear local variable
}

void setup() {
  // put your setup code here, to run once:
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(12, OUTPUT);
  
  digitalWrite(12,LOW);
  digitalWrite(BUILTIN_LED, HIGH);

  myservo.attach(4);  //attaches the servo on GIO2 to the servo object.
  myservo.write(100); //Initial servo position.
  
  Serial.begin(115200);
  setup_wifi();
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected()){
    reconnect();
  }
  client.loop();
  delay(100);
}
