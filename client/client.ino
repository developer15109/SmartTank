/*
 * Code Is Written By: Umesh Kumar @ 2017
 */

// This is the Server File Which Basically Read the Data.
// All the required header files are included here

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <NewPing.h>
#include <EEPROM.h>

//Values that to be set at the time of installation

// Login Credentials

const char* ssid="IoT";
const char* pass="IoT@2017";    

// Global Variable

// All are in cm

#define radius 12.0                 //Radius of the tank   in cm                  
#define StopDistance 6              // Stoping distance of the motor 
#define StartDistance 23             // starting distance of the motor
#define EndPoint 30                 // End point of the tank  // that is the total height of the tank..

float VolumeOfBucket=12.50;          // Total volume of the tank this is to be calculated

// Finish Below this nothing need to be changed..!!

unsigned int Timeout=600;       // that is wait for 10 minutes before restarting the device

#define TRIGGER_PIN 12
#define ECHO_PIN 14
#define MAX_DISTANCE 35

int port=9090;                        // port number
IPAddress server(192,168,4,1);       // the fix IP address of the server
WiFiClient client;

// Indicators
int Yellow=4;         // Yellow( Device On/Off State )  D2 Pin on the Device
int Green=13;         // Green ( Motor is On ) D5 pin on the device
int Blue=5;           // Blue  ( Internet Connection has setup Successfully ) D1 Pin on the Dev   ice

// State of the motor at the client level
int token=-1;
int addressOfState=0; // starting address of the memory to store the address

// some variable to hold the volume information of the tank

float Volume=0.0;               // Temporary Variable to hold volume of the tank

float finalVolume=0.0;          // Remaining final volumet inside the tank
 
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

unsigned int start;


void setup() {
  delay(2000);
  // Pin Configurations
  PinIntialise();
 
  //  Serial Monitor configuration
  Serial.begin(115200);
  while(!Serial);
   
  // Intial Delay for the Setup
   
   
   
   unsigned long NewTime=(millis()/1000)+60*2;    // Wait fot two minutes if the connection does not start please restart the device.
   
   while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    if((millis()/1000)>NewTime)
    {
      ESP.restart();
    }
  }  
 
  Serial.println("");
  Serial.println("Connected to wifi");
  
  digitalWrite(Yellow,HIGH);           //Device is ON
  EEPROM.begin(512);
  
  int value=EEPROM.read(addressOfState);
  
  if(value==-1)
  {
    Serial.println("No state is there intially");
  }
  else if(value==0)
  {
    Serial.println("Motor is OFF");
    digitalWrite(Green,LOW);
  }
  else if(value==1)
  {
    Serial.println("Motor Was ON, Reseting all to default");
    digitalWrite(Green,LOW);
  }
  else {
    Serial.println("Nothing Cant be said now....");
  }

  start=millis()/1000;
}


void PinIntialise()
{
   pinMode(Yellow,OUTPUT);
   pinMode(Green,OUTPUT);
   pinMode(Blue,OUTPUT);
   
   pinMode(TRIGGER_PIN,OUTPUT);
   pinMode(ECHO_PIN,INPUT);
}

void WifiSetup(){
   delay(1000);
   Serial.println("Welcome Jagat This is the web server created by ESP8266 Module");
   Serial.println();

   // WiFi Configuration
   Serial.println("Configuring the Wifi settings...");
   Serial.println("Now Connecting to ssid: ");
   Serial.println(ssid);
      
   WiFi.mode(WIFI_STA);
   WiFi.begin(ssid,pass);
}

void loop() { 
   
  if(client.connect(server,port)){
    Serial.println("Client connected to the server");
    digitalWrite(Blue,HIGH);                            // Turn On the Blue Light As We have Successfully Connected to the internet
  }

  while(!client.connected())
   {
    Serial.println("Client is not connected to the server");          // that is server has stoped.!
    digitalWrite(Blue,LOW);
    //digitalWrite(Red,LOW);
    digitalWrite(Green,LOW);
    client.connect(server,port);
    if(token==-1)
    { 
      Serial.println("Token is set to -1");
      EEPROM.write(addressOfState,token);
    }
    if((millis()/1000)>start+Timeout)                // Okay Restart the device after the 10 minutes of the start
    {
     digitalWrite(Blue,LOW);
     //digitalWrite(Red,LOW);
     digitalWrite(Green,LOW); 
    ESP.restart();
    }
    delay(10);
   }
   
  if (client){                            // Check if a client has connected
    digitalWrite(Blue,HIGH);
    token=0;
     while(client.connected())
      {
       unsigned int distance = sonar.ping_cm();
       Serial.print(distance);
       Serial.println("cm");
    
        Volume=(3.14*radius*radius*distance)/1000.0;
        finalVolume=VolumeOfBucket - Volume;
        
        if(distance<=StopDistance)                               //  sample test STopDistance is 10
        {
         token=0;

         EEPROM.write(addressOfState,token);
         client.println("OFF");                    // Turn the motor OFF
         client.println(finalVolume);              // Also send the water in the tank 

         digitalWrite(Green,LOW);
         /*digitalWrite(Red,HIGH);
         delay(500);
         digitalWrite(Red,LOW);
         delay(500);
         */
         Serial.print("The volume of the tank is :  ");
         Serial.print(finalVolume);
         Serial.println(" lt");
          
       }

       if(distance >StopDistance && distance<(StopDistance+5))              //  sample test StopDistance 10 and  15
       {
        client.println(finalVolume);
        Serial.print("The volume of the tank is :  ");
        Serial.println(finalVolume);
        //Serial.println("You are in the Do nothing Condition");
        delay(50);
       }

       if(distance >=(StopDistance+6) && distance <=EndPoint)
         { 
           if(distance>=StartDistance)
          {
           token=1;

           EEPROM.write(addressOfState,token);
           client.println("ON");
           client.println(finalVolume);

           digitalWrite(Green,HIGH);    // SWitch On the motor
          // Serial.println("Bucket is Empty So motor has been switched On");
           Serial.print("The volume of the tank is :  ");
           Serial.print(finalVolume);
           Serial.println(" lt");
           delay(500); 
          }
          else
          {
           client.println(finalVolume);
   
           Serial.print("The volume of the tank is :  ");
           Serial.print(finalVolume);
           Serial.println(" lt");
           delay(500);
          }
       }
    }                 
  }
  else{   
    Serial.println("Disconnected the Client");
    //digitalWrite(Red,LOW);
    digitalWrite(Green,LOW);
    digitalWrite(Blue,LOW);            
  }
}

