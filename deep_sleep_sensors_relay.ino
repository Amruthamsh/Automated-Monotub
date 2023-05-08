#include <SoftwareSerial.h>
#include <stdio.h>
#include <stdlib.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

const byte rxPin = 3;
const byte txPin = 2;

// Set up a new SoftwareSerial object
SoftwareSerial Serial1 (rxPin, txPin);

bool esp32_goingToSleep = false;

//CO2
const int pwmPin = 5;

// Set DHT pin for TEMP-HUM
#define DHTPIN 4
#define DHTTYPE DHT11   // DHT 11 
// Initialize DHT sensor for normal 16mhz Arduino:
DHT dht = DHT(DHTPIN, DHTTYPE);

bool dark_phase = true;

/*Relay
1 - Top Layer fans (2 in number): oppositely wired fans - one fans forcing air in, one fan forcing air out
2 - Humidifier 
3 - Humidifier Fan
4 - Peltier
5 - Peltier fans(2 in number)
6 - 
*/
#define RELAY1 6 
#define RELAY2 7 
#define RELAY3 8 
#define RELAY4 9 
#define RELAY5 10 

void setup(){
  Serial1.begin(115200);
  pinMode(pwmPin, INPUT_PULLUP);
  // Setup sensor:
  dht.begin();

  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(RELAY1, OUTPUT); 
  pinMode(RELAY2, OUTPUT); 
  pinMode(RELAY3, OUTPUT); 
  pinMode(RELAY4, OUTPUT); 
  pinMode(RELAY5, OUTPUT);
}

void loop() {
  if(Serial1.available()){
    String buf = "";
    while(Serial1.available()){
      char c = Serial1.read();   //save the arrived charcater in buf[] array
      buf.concat(c);      
      if(c == 'H')
      {
        digitalWrite(LED_BUILTIN, HIGH); 
        dark_phase = false; 
        break; 
      }
      if(c == 'D')
      {
        digitalWrite(LED_BUILTIN, LOW); 
        dark_phase = true; 
        break; 
      }       
    }
    /*
    if (buf.compareTo("aaaa") == 0) //check if arrived string is asdf
    {
      Serial.println('a');   //string matched; send a to OutputBox of Serial Monitor
      //digitalWrite(LED_BUILTIN, HIGH);      
      esp32_goingToSleep = false;  
    }

    if (buf.compareTo("ssss") == 0) //check if arrived string is asdf
    {
      Serial.println('s');   //string matched; send a to OutputBox of Serial Monitor
      //digitalWrite(LED_BUILTIN, LOW);      
      esp32_goingToSleep = true;  
    }

    if (buf.compareTo("DARK") == 0) //check if arrived string is asdf
    {
      //Serial.println('s');   //string matched; send a to OutputBox of Serial Monitor   
      digitalWrite(LED_BUILTIN, LOW); 
      dark_phase = true;    
    }
    
    if (buf.compareTo("HARVEST") == 0) //check if arrived string is asdf
    {
      //Serial.println('s');   //string matched; send a to OutputBox of Serial Monitor  
      digitalWrite(LED_BUILTIN, HIGH); 
      dark_phase = false; 
    }
*/    
  }  
  
  int ppmPWM = readPPMPWM();
  //Serial.print(ppmPWM); 
  //Serial.print("PPM\t");
  //int ppmPWM = 4500;
    

  char buffer [30], buff_t [10], buff_h[10];
  // Read the humidity in %:
  float h = dht.readHumidity();
  // Read the temperature as Celsius:
  float t = dht.readTemperature();


  if(!esp32_goingToSleep)
  {
    dtostrf(t, 4, 2, buff_t); //4 - width, 2 - precision
    dtostrf(h, 4, 2, buff_h); 
    
    itoa (ppmPWM,buffer,10); //CO2
    strcat(buffer, "&");
    strcat(buffer, buff_t);
    strcat(buffer, "&");
    strcat(buffer, buff_h);
    strcat(buffer, "=");
    Serial1.write(buffer);      
  }    

  
  if(dark_phase){
    if(t > 28)
    {
      digitalWrite(RELAY4,HIGH); //peltier
      digitalWrite(RELAY5,HIGH); 
    }  

    if(t < 22)
    {
      digitalWrite(RELAY4,LOW); //peltier
      digitalWrite(RELAY5,LOW); 
    }

    if(h > 70)
    {
      //Humidifier
      digitalWrite(RELAY2,LOW); 
      digitalWrite(RELAY3,LOW); 
    }

    if(h < 60)
    {
      //Humidifier
      digitalWrite(RELAY2,HIGH); //continuous
      digitalWrite(RELAY3,HIGH);  //30 sec wait - 1 minute ON
    }
  }
  else{
    if(t > 24)
    {
      digitalWrite(RELAY4,HIGH); //peltier
      digitalWrite(RELAY5,HIGH); 
    }  

    if(t < 20)
    {
      digitalWrite(RELAY4,LOW); //peltier
      digitalWrite(RELAY5,LOW); 
    }

    //CO2 < 600-1100 - fans
    //Hum 85 to 95 - fans and hum
    if(ppmPWM > 950)
    {
        digitalWrite(RELAY1,HIGH); 
    }

    if(ppmPWM < 550)
    {
        digitalWrite(RELAY1,LOW); 
    }

    if(h > 95)
    {
      digitalWrite(RELAY2,LOW); 
      digitalWrite(RELAY3,LOW);
    }

    if(h < 75)
    {
      //Humidifier
      digitalWrite(RELAY2,HIGH); //continuous
      digitalWrite(RELAY3,HIGH);  //30 sec wait - 1 minute ON
    }
  }  
}


int readPPMPWM() {
  while (digitalRead(pwmPin) == LOW) {}; // wait for pulse to go high
  long t0 = millis();
  while (digitalRead(pwmPin) == HIGH) {}; // wait for pulse to go low
  long t1 = millis();
  while (digitalRead(pwmPin) == LOW) {}; // wait for pulse to go high again
  long t2 = millis();
  long th = t1-t0;
  long tl = t2-t1;
  long ppm = 5000L * (th - 2) / (th + tl - 4);
  while (digitalRead(pwmPin) == HIGH) {}; // wait for pulse to go low
  delay(10); // allow output to settle.
  return int(ppm);
}
