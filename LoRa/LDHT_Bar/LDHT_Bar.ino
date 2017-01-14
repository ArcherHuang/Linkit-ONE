#include "LDHT.h"
#include <Grove_LED_Bar.h>
Grove_LED_Bar bar(9, 8, 0);

#define DHTPIN 8      
#define DHTTYPE DHT22  

LDHT dht(DHTPIN,DHTTYPE);

float tempC = 0.0, Humi = 0.0;
float compareTemp = 0.0;
unsigned int ledLevel = 0;

void setup()
{
    Serial.begin(9600);
    dht.begin();
    bar.begin();
    bar.setLevel(0);
}

void loop()
{
    if(dht.read())
    {
        tempC = dht.readTemperature();
        Humi = dht.readHumidity();

        Serial.println("------------------------------");
        Serial.print("Temperature Celcius = ");
        Serial.print(dht.readTemperature());
        Serial.println("C");
        Serial.print("Temperature Fahrenheit = ");
        Serial.print(dht.readTemperature(false));
        Serial.println("F");
        Serial.print("Humidity = ");
        Serial.print(dht.readHumidity());
        Serial.println("%");

        if(tempC > compareTemp){
          Serial.print("setGreenToRed(0)");
          ledLevel++;
          for (int i = ledLevel; i <= ledLevel; i++) {
            Serial.println("0");
            bar.setLed(i, 1);
            delay(1000);
          }
        }else if (tempC < compareTemp){
          Serial.print("setGreenToRed(1)");
          ledLevel--;
          for (int i = ledLevel; i <= ledLevel; i++) {
            bar.setLed(i, 1);
            delay(1000);
          }
          
        }else{
          Serial.print("compareTemp = ");
          Serial.println(compareTemp);
          Serial.print("tempC = ");
          Serial.println(tempC);
       }   
      compareTemp = tempC;
    }
    delay(1000);
}
