#include <Grove_LED_Bar.h>
Grove_LED_Bar bar(9, 8, 0);

const int TouchPin = 2; 

void setup() {
  // put your setup code here, to run once:
  bar.begin();
  pinMode(TouchPin, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:

  int sensorValue = digitalRead(TouchPin); 
  if(sensorValue==1)
  {
    for (int i = 0; i <= 10; i++)
    {
      bar.setLevel(i);
      delay(500);
    }
    //bar.setLevel(0);
    //bar.setGreenToRed(0);
    //delay(500);
  }else{
    bar.setLevel(10);
  }

}
