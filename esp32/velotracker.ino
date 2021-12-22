/*
 * velotracker01 this file is intended to be run on the esp32 for the biketracker
 * the sensors are: neo-6m and the Sim800l
 */


/*
 * Gps and other stuff
 */
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <HTTPClient.h>

#define TINY_GSM_MODEM_SIM800
#include <TinyGSM.h>

// These are some Handy definitions
#define uS_TO_S_FACTOR 1000000ULL
#define TIME_TO_SLEEP 20

#define gpsRX 34
#define gpsTX 35
#define gpsGsmBaud 9600

/*
 * TinyGsm related stuff
 */
#define gsmRX 26
#define gsmTX 27
#define TINY_GSM_MODEM_800
#include <TinyGsmClient.h>
#include <PubSubClient.h>
SoftwareSerial ss(gsmRX, gsmTX);
SoftwareSerial gpsSS(gpsRX, gpsTX);

// #define SerialMon Serial
#define TINY_GSM_DEBUG SerialMon
#define TINY_GSM_USE_GPRS true

// Gprs credentials
const char apn[] = "";
const char gprsUser[] = "";
const char gprsPass[] = "";

// MQTT credentials
const char* broker = "";
const char* topicTest = "";

TinyGsm modem(ss);
TinyGsmClient client(modem);
PubSubClient mqtt(client);


//Objects for the gps
TinyGPSPlus gps;
double lat;
double lng;


void setup() {
  Serial.begin(115200); // This is for debug purposes
  Serial.println("Program starts");

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

   /*
    * The Location is gathered from the GPS
    */
   gpsSS.begin(gpsGsmBaud);
   gpsSS.listen();
   delay(2000);

  // Get the location from the gps
  while (!gps.location.isValid())
    while (gpsSS.available() > 0) 
        gps.encode(gpsSS.read());


  Serial.print("lat: ");
  Serial.print(gps.location.lat());
  Serial.println();
  gpsSS.end();
 

   /*
    * The data is transferred with the GSM-Module
    */
  // The Module starts up and connects to the network
  Serial.println("beginning with the GSM-Module");
   
  ss.begin(gpsGsmBaud);
  ss.listen();
  Serial.println("Initializing Modem...");
  modem.restart();
  ss.write("AT+CFUN=1");

  Serial.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    Serial.println(" fail");
    delay(1000);
    Serial.println("returning");
    return;
  }
  Serial.println(" success");
  if (modem.isNetworkConnected()) { Serial.println("Network connected"); }

  Serial.print(F("Connecting to "));
  Serial.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    Serial.println(" fail");
    ESP.restart();
    delay(10000);
    return;
  }
  Serial.println(" success");

  if (modem.isGprsConnected()) { Serial.println("GPRS connected"); }

  // The JSON string is concatenated and send to the broker
  Serial.println("Setting broker...");
  mqtt.setServer(broker, 51883);
  mqtt.setCallback(mqttCallback);

  boolean test = mqttConnect();

  Serial.println("publishing");
  String msgString = "{\"trck\":\"trck01\", \"lat\":" + String(gps.location.lat(),6) + ", \"lng\":" + String(gps.location.lng(),6) + "}";
  char msg[60];
  msgString.toCharArray(msg, 60);
  mqtt.publish(topicTest, msg);
  Serial.println("published");

  Serial.println("setting to minimal functionality mode");
  ss.write("AT+CFUN=0");
  

  // going to deep sleep
  esp_deep_sleep_start();
}

void mqttCallback(char* topic, byte* payload, unsigned int len) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.write(payload, len);
  Serial.println();
}

boolean mqttConnect() {
  Serial.print("Connecting to ");
  Serial.print(broker);

  // Connect to MQTT Broker
  boolean status = mqtt.connect("GsmClientTest");

  // Or, if you want to authenticate MQTT:
  // boolean status = mqtt.connect("GsmClientName", "mqtt_user", "mqtt_pass");

  if (status == false) {
    Serial.println(" fail");
    return false;
  }
  Serial.println(" success");
  mqtt.subscribe(topicTest);
  return mqtt.connected();
}

void loop() {
  // We don't need this because we use deep sleep
}
