#include "LDHT.h"
#define DHTPIN 2      
#define DHTTYPE DHT22  

LDHT dht(DHTPIN,DHTTYPE);

#include <HttpClient.h>
#include <LWiFi.h>
#include <LWiFiClient.h>
#define WIFI_AP "wifi_03" //my wifi ap
#define WIFI_PASSWORD "wnec1234" //my wifi ap password
#define WIFI_AUTH LWIFI_WPA //the kind of authority of my wifi which is WPA2
#define DEVICEID "DBKHFNIw" //the device id given by cloud sandbox
#define DEVICEKEY "8fszdReA51m0vRjq" //the device key given by cloud sandbox
#define SITE_URL "api.mediatek.com" //the site of the API

LWiFiClient c2;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  dht.begin();

  LWiFi.begin();
  while(!Serial) delay(1000); /* comment out this line when Serial is not present, ie. run this demo without connect to PC */

  Serial.println("Connecting to AP");
  while (0 == LWiFi.connect(WIFI_AP, LWiFiLoginInfo(WIFI_AUTH, WIFI_PASSWORD)))
  {
    delay(1000);
  }
  
  Serial.println("calling connection");

  while (!c2.connect(SITE_URL, 80))
  {
    Serial.println("Re-Connecting to WebSite");
    delay(1000);
  }
  delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(dht.read())
  {
    Serial.println("------------------------------");
    Serial.print("Temperature Celcius = ");
    Serial.print(dht.readTemperature());
    Serial.println("C");
    Serial.print("Humidity = ");
    Serial.print(dht.readHumidity());
    Serial.println("%");

    uploadstatus();
  }

  delay(1000);
}

void uploadstatus(){
  
  Serial.println("calling connection"); 
  String uploadHumidity = "Humidity,," + String(dht.readHumidity());
  String uploadTemperature = "Temperature,," + String(dht.readTemperature());

  while (!c2.connect(SITE_URL, 80))
  {
    Serial.println("Re-Connecting to WebSite");
    delay(1000);
  }
  delay(100);
  
  String uploadData =  uploadHumidity + "\n" +
                       uploadTemperature + "\n";

  HttpClient http(c2);
  c2.print("POST /mcs/v2/devices/");
  c2.print(DEVICEID);
  c2.println("/datapoints.csv HTTP/1.1");
  c2.print("Host: ");
  c2.println(SITE_URL);
  c2.print("deviceKey: ");
  c2.println(DEVICEKEY);
  c2.print("Content-Length: ");
  c2.println(uploadData.length());
  c2.println("Content-Type: text/csv");
  c2.println("Connection: close");
  c2.println();
  c2.println(uploadData);
  //upload the data
  
//  delay(500);
//
//  int errorcount = 0;
//  while (!c2.available())
//  {
//    Serial.print("waiting HTTP response: ");
//    Serial.println(errorcount);
//    errorcount += 1;
//    if (errorcount > 10) {
//      c2.stop();
//      Serial.println("uploadStatus failed");
//      return;
//    }
//    delay(100);
//  }
//  int err = http.skipResponseHeaders();
//
//  int bodyLen = http.contentLength();
//  Serial.print("Content length is: ");
//  Serial.println(bodyLen);
//  Serial.println();
//  while (c2)
//  {
//    int v = c2.read();
//    if (v != -1)
//      Serial.print(char(v));
//    else
//    {
//      Serial.println("no more content, disconnect");
//      c2.stop();
//    }
//  }
}
