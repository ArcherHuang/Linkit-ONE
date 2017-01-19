#include "LDHT.h"

#define DHTPIN 7          // what pin we're connected to
#define DHTTYPE DHT22     // using DHT22 sensor
#define FAN_PORT 10
#define LED_PORT 6

//functions defination
LDHT dht(DHTPIN, DHTTYPE);
void serial_one_msg();
void LoRa_control_activate();

//Glabal valuables
float tempC = 0.0, Humi = 0.0;
char temp_humi[8];
char readcharbuffer[20];
int readbuffersize;
char temp_input;

int serial_delay_count = 0;
int read_flag = 1;

String LoRa_DL_Data = "";
char LoRa_Control;

void setup(){
  Serial.begin(9600);
  Serial1.begin(9600);

  dht.begin();
  pinMode(LED_PORT, OUTPUT);
  pinMode(FAN_PORT, OUTPUT);

  Serial.print(DHTTYPE);
  Serial.println("test!");
}

void loop(){

  Serial.println("Getting DHT temperature and humidity information.");
  while(dht.read()<=0){
  }
  if (dht.read()){
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

    Serial.print("HeatIndex = ");
    Serial.print(dht.readHeatIndex(tempC, Humi));
    Serial.println("C");

    Serial.print("DewPoint = ");
    Serial.print(dht.readDewPoint(tempC, Humi));
    Serial.println("C");

    sprintf(temp_humi,"%05.2f", Humi);
    Serial.println(Humi);
    Serial.println(temp_humi);

    Serial.println("Ready to Send");
    Serial.print("AT+DTX=11,\"T");
    Serial.print(tempC);
    Serial.print(temp_humi);
    Serial.println("\"");

    Serial1.print("AT+DTX=11,\"T");
    Serial1.print(tempC);
    Serial1.print(temp_humi);
    Serial1.println("\"");

    //Serial1.println("AT+DTX=11,\"12345ABCdef\"");
  }


  Serial.println("Reading the mcu message on Node");
  serial_one_msg();
  Serial.println("End of Node response");
  delay(1000);

//Dectect there are dl packets or not if there is no message from Node
// It means no data from dl
// If no dl data from GW do not send AT+DRX? command or node will not send data to GW next time
// It should be bug in MCU code
  while(Serial1.available()<=0){
    serial_delay_count++;
    delay(1000);
    if(serial_delay_count == 10){
      read_flag = 0;
      serial_delay_count = 0;
      break;
    }
  }

  if(read_flag == 0){
    Serial.println("No downlink Message available");
    read_flag = 1;
  }else{
    readbuffersize = Serial1.available();
    Serial.print(readbuffersize);
    while(readbuffersize){
      temp_input = Serial1.read();
      Serial.print(temp_input);
      readbuffersize--;
    }
    //Wait until io is Ready
    delay(5000);
    //receiving message
    Serial1.println("AT+DRX?");
  }


  //Wait until io is Ready
  delay(5000);


  while(Serial1.available()<=0){
    serial_delay_count++;
    delay(1000);
    if(serial_delay_count == 10){
      read_flag = 0;
      serial_delay_count = 0;
      break;
    }
  }

  if(read_flag == 0){
    read_flag = 1;
  }else{
    readbuffersize = Serial1.available();
    Serial.print("Data from serial1 size is: ");
    Serial.println(readbuffersize);
    LoRa_DL_Data = Serial1.readStringUntil(',');
    Serial.print(LoRa_DL_Data);
    LoRa_Control = Serial1.read();
    Serial.print(LoRa_Control);
    while(Serial1.available()){
      LoRa_DL_Data = Serial1.readString();
      Serial.print(LoRa_DL_Data);
    }
  }

  // control hardware functions
  LoRa_control_activate();


  delay(45000);

  Serial.println("TX relay done should show up things 5");
  serial_one_msg();
  delay(1000);

//  dl_msg();*/
}

void serial_one_msg(){
  String serial1_temp_reading = "";

  while(Serial1.available()<=0){
    serial_delay_count++;
    delay(1000);
    if(serial_delay_count == 10){
      read_flag = 0;
      serial_delay_count = 0;
      break;
    }
  }

  if(read_flag == 0){
    read_flag = 1;
  }else{
    readbuffersize = Serial1.available();
    Serial.print("Data from serial1 size is: ");
    Serial.println(readbuffersize);
    while(Serial1.available()){
      serial1_temp_reading = Serial1.readString();
      Serial.print(serial1_temp_reading);
    }
  }
}

void LoRa_control_activate(){
  if(LoRa_Control == '0'){
    digitalWrite(LED_PORT, HIGH);
    digitalWrite(FAN_PORT, HIGH);
    Serial.println("All OFF");
  }else if(LoRa_Control == '1'){
    digitalWrite(LED_PORT, LOW);
    digitalWrite(FAN_PORT, HIGH);
    Serial.println("FAN off LED on");
  }else if(LoRa_Control == '2'){
    digitalWrite(LED_PORT, HIGH);
    digitalWrite(FAN_PORT, LOW);
    Serial.println("FAN On LED off");
  }else if(LoRa_Control == '3'){
    digitalWrite(LED_PORT, LOW);
    digitalWrite(FAN_PORT, LOW);
    Serial.println("All devices ON");
  }
}
