#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 8

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// arrays to hold device address
DeviceAddress insideThermometer;

int tempF ;  //Temperature in °F

int LED_Pin = 7;  //Heizungs LED
int Heater_Pin = 4; 
int Pump_Pin = 5;
int TempOK_Pin = 6;
int BottleTempMin = 86;
int BottleTempMax = 104;
int BottleTemp = 97;
int hysterese = 2; // in °F
const int analogInPin = A0;
int sensorValue = 0;

int HeaterIntervall = 100;  // 360 entspricht 3 min auslesezeit Temperatur sensor ca 500 ms 
int HeaterDauer = 20;  // 20 entspricht 10s auslesezeit Temperatur sensor ca 500 ms
int HeaterCounter = 0;
int coldstart = 0; // 


int tempsim = 80;
//#define simulation
//#define logging
//#define debug
void setup(void)
{
  // start serial port
  Serial.begin(9600);
  
  pinMode(LED_Pin, OUTPUT);
  pinMode(Heater_Pin, OUTPUT);  
  pinMode(TempOK_Pin, OUTPUT);
  pinMode(Pump_Pin, OUTPUT);
  
  
  
  Serial.println("Dallas Temperature IC Control Library Demo");

  // locate devices on the bus
  Serial.print("Locating devices...");
  sensors.begin();
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: "); 
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");
  
  // assign address manually.  the addresses below will beed to be changed
  // to valid device addresses on your bus.  device address can be retrieved
  // by using either oneWire.search(deviceAddress) or individually via
  // sensors.getAddress(deviceAddress, index)
  //insideThermometer = { 0x28, 0x1D, 0x39, 0x31, 0x2, 0x0, 0x0, 0xF0 };

  // Method 1:
  // search for devices on the bus and assign based on an index.  ideally,
  // you would do this to initially discover addresses on the bus and then 
  // use those addresses and manually assign them (see above) once you know 
  // the devices on your bus (and assuming they don't change).
  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0"); 
  
  // method 2: search()
  // search() looks for the next device. Returns 1 if a new address has been
  // returned. A zero might mean that the bus is shorted, there are no devices, 
  // or you have already retrieved all of them.  It might be a good idea to 
  // check the CRC to make sure you didn't get garbage.  The order is 
  // deterministic. You will always get the same devices in the same order
  //
  // Must be called before search()
  //oneWire.reset_search();
  // assigns the first address found to insideThermometer
  //if (!oneWire.search(insideThermometer)) Serial.println("Unable to find address for insideThermometer");

  // show the addresses we found on the bus
  Serial.print("Device 0 Address: ");
  printAddress(insideThermometer);
  Serial.println();

  // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
  sensors.setResolution(insideThermometer, 12);
 
  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(insideThermometer), DEC); 
  Serial.println();
}

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress)
{
  // method 1 - slower
  //Serial.print("Temp C: ");
  //Serial.print(sensors.getTempC(deviceAddress));
  //Serial.print(" Temp F: ");
  //Serial.print(sensors.getTempF(deviceAddress)); // Makes a second call to getTempC and then converts to Fahrenheit

  // method 2 - faster
  float tempC = sensors.getTempC(deviceAddress);
  #ifdef debug 
  Serial.print("Temp C: ");
  Serial.print(tempC);
  Serial.print(" Temp F: ");
  #endif
 tempF=DallasTemperature::toFahrenheit(tempC);
 
 #ifdef simulation
 if (digitalRead(Heater_Pin)== HIGH ){tempsim++; }
  else
  {tempsim--;}
   tempF=tempsim;
 delay (500); 
 #endif  
 #ifdef debug 
   Serial.println(DallasTemperature::toFahrenheit(tempC)); // Converts tempC to Fahrenheit
 #endif  
}

void loop(void)
{ 
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
#ifdef debug 
  Serial.print("Requesting temperatures...");
  #endif
  sensors.requestTemperatures(); // Send the command to get temperatures
#ifdef debug 
  Serial.println("DONE");
#endif
  // It responds almost immediately. Let's print out the data
  printTemperature(insideThermometer); // Use a simple function to print out the data
  
 /*
Bottle Temp°F  Bottle Pressure (psi)
40                    520
50                    590
60                    675 
70                    760
80                    865
85                    950
97                    1069

*/ 

// read the analog in value:
  sensorValue = analogRead(analogInPin);            
  // map it to the range of the analog out:
  BottleTemp = map(sensorValue, 0, 1023,BottleTempMin ,BottleTempMax);  

// hochheizen bis tem das erste mal erreicht----------------------
if (tempF < (BottleTemp) && coldstart == 0 ){ 
  digitalWrite(LED_Pin, HIGH); 
  digitalWrite(Heater_Pin, HIGH); 
  }
  else
 { 
 coldstart = 1;  
 digitalWrite(LED_Pin, LOW); 
 digitalWrite(Heater_Pin, LOW); 
 }
// ---------------------- 
 


// erhaltungsheizung----------------------
if (tempF < (BottleTemp-hysterese) && coldstart == 1){ 
  coldstart = 0;
  }

// Pumpensteuerung ----------------------  
 
  if ( HeaterCounter >= HeaterIntervall && digitalRead(Heater_Pin)== HIGH && tempF < 82) {
    digitalWrite(Pump_Pin, HIGH);
    HeaterCounter = 0;
    }
    
   if ( HeaterCounter >= HeaterDauer && digitalRead(Pump_Pin)== HIGH && tempF < 82 ) { 
    digitalWrite(Pump_Pin, LOW);
    HeaterCounter = 0;
    }
    if (  digitalRead(Heater_Pin)== HIGH && tempF >= 82) {
    digitalWrite(Pump_Pin, HIGH);
    
    }
    
   if ( digitalRead(Heater_Pin)== LOW && tempF >= 82 ) { 
    digitalWrite(Pump_Pin, LOW);
   
    }
 
 
 
if (tempF >= (BottleTemp - hysterese) && tempF <= (BottleTemp + hysterese )){digitalWrite(TempOK_Pin, HIGH);}
else
{
 digitalWrite(TempOK_Pin, LOW);
 }
 
 #ifdef logging
Serial.print(millis()); 
Serial.print(";"); 
Serial.print(BottleTemp);
Serial.print(";");
Serial.print(tempF);
Serial.print(";");
Serial.print(digitalRead(Heater_Pin));
Serial.print(";");
Serial.print(digitalRead(TempOK_Pin));
Serial.print(";");
Serial.println(digitalRead(Pump_Pin));
#endif 


#ifdef debug
 Serial.print("Kaltstart: ");
 Serial.println(coldstart);
 Serial.print("Ziel Temperatur: ");
 Serial.println(BottleTemp);
 Serial.print("Aktulle Temperatur: ");
 Serial.println(tempF);
 Serial.print("Heizung: ");
 Serial.println(digitalRead(Heater_Pin));
 Serial.print("Temperatur OK: ");
 Serial.println(digitalRead(TempOK_Pin));
 Serial.print("Pumpe: ");
 Serial.println(digitalRead(Pump_Pin));
 Serial.print("Counter: ");
 Serial.println(HeaterCounter);
 #endif
 HeaterCounter ++;
 
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
  
  
}
