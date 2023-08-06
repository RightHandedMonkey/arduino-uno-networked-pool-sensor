/*
This example creates a client object that connects and transfers
data using always SSL.

It is compatible with the methods normally related to plain
connections, like client.connect(host, port).

Written by Arturo Guadalupi
last revision November 2015

*/

#include <SPI.h>
#include <WiFiNINA.h>
//Time protocol
#include <NTPClient.h>

#include "arduino_secrets.h" 
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key index number (needed only for WEP)

int status = WL_IDLE_STATUS;
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
char server[] = "io.adafruit.com";    // name address for Google (using DNS)

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiSSLClient client;
WiFiUDP ntpUDP;
long etOffsett = -14400;
float poolTemp=32.0;
String timer = "";

// By default 'pool.ntp.org' is used with 60 seconds update interval and
// no offset
NTPClient timeClient(ntpUDP, etOffsett);

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(5000);
  }
  Serial.println("Connected to WiFi");
  printWiFiStatus();

  // Serial.println("\nStarting connection to server...");
  // connectionTest();
  timeClient.begin();
  delay(2000);
  timeClient.update();
}

void loop() {
  readTemperature();
  Serial.print("Time: ");
  timer = timeClient.getDay() +" "+timeClient.getFormattedTime();
  Serial.println(timer);
  Serial.println("Temp to send: "+String(poolTemp));

  String json_data = "{\n  \"value\": "+ String(poolTemp) +"\n}";
  String auth = "X-AIO-Key: "+IO_KEY;
  String target = "POST /api/v2/"+IO_USERNAME+"/feeds/"+FEED_VALUE+"/data HTTP/1.1";
  postData(json_data, target, auth);
  delay(500);
  readData();
  String time_data = "{\n  \"value\": \""+timer+"\"\n}";
  target = "POST /api/v2/"+IO_USERNAME+"/feeds/"+FEED_DATETIME+"/data HTTP/1.1";
  postData(time_data, target, auth);
  delay(500);
  readData();


  delay(10000);
}

void postData(String data, String destination, String auth) {
  client.stop();
  // if there's a successful connection:
  if (client.connect(server, 443)) {
    outputLines(destination);
    outputLines("Host: io.adafruit.com");
    if (auth.length() > 1) {
      outputLines(auth);
    }
    outputLines("Content-Type: application/json");
    outputLines("Content-Length: " + String(data.length())); //ANSWER: body.length() needs to be wrapped as a string, see above
    outputLines("Connection: close");
    outputLines(""); // end HTTP request header
    outputLines(data);
  } else {
    Serial.println("Could not establish a connection...");
  }
}

void readTemperature() {

}

void outputLines(String string){
  if (string.length() < 1) {
    Serial.println();
    client.println();
  } else {
    Serial.println(string);
    client.println(string);
  }
}

void connectionTest() {
  if (client.connect(server, 443)) {
    Serial.println("connected to server");
    // Make a HTTP request:
    client.println("GET / HTTP/1.1");
    client.println("Host: io.adafruit.com");
    client.println("Connection: close");
    client.println();
  }
}

void readData() {
    // if there are incoming bytes available
  // from the server, read them and print them:
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting from server.");
    client.stop();
  }
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}


