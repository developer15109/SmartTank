
/*
  Written By:
  Umesh kumar
*/

#include <ArduinoHttpClient.h>
#include <b64.h>
#include <HttpClient.h>

// The below code uses the amazon Web Services
#include <ESP8266WiFi.h>
#include <SPI.h>
//#include <aJSON.h>
#include <EEPROM.h>
#include <DNSServer.h>  

#include <WiFiClient.h>
#include <WiFiManager.h>  
#include <ArduinoJson.h>

#include <ESP8266HTTPClient.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 16, 2);

//Login Credentials 

char const* name=  "Umesh";
char const* ssid = "IoT";                 // SSID of your home WiFi
char const* pass = "IoT@2017";            // password of your home WiFi

char const* AWS_ssid     = "AshutoshAnand.com";
char const* AWS_password = "ashutosh15018";

const char* host = "192.168.4.1"; 

char const* app_id="0001";
char const* app_secret="2001";
int type=1;

char const* fingerPrint="bfcdef178809461464c5d9de44f9fdc63dafb9a3";

// Global Variables

#define TimeOut 900         // that is after 15 minutes automatically swicth off the motor 60*15
#define WaitingTime 600     // 60*10 Wait for atleast 10 minutes before restart the ESP8266 board 60*10

// Below this no need to change anything

int port=9090;
WiFiServer server(port); //just pick any port number you like

WiFiClient client;
WiFiManager wifiManager;

// some variables

// Indicators
int Blue=5;             // Blue  (Internet Connection has setup Successfully ) D1 Pin on the Device
int MainSwitch=4;       // Yellow( Device On/Off State )  D2 Pin on the Device
int relay=12;           // Relay Pin D8 Pin on the Device
int Green=14;         // Green for the motor state

float TimeBetON_OFF = 0.0;
String waterLevel    = "";
String FinalState   = "";
String Quality      = "Not Given";
String Message      = "Decision for Message is not decided yet";
unsigned int NumberOfTimes=0;

int Indicator;
int postFlag=-1;

// States of motor
int On=0;
int Off=0;

// token variable to hold the current state of the motor i.e token=0 motor if off and token=1 motor is on
int token=-1;
int value;
// timer variable to keep counting the duration for which the motor runs
unsigned int timer=0;

  // local variable to store the current time value
unsigned int t;

// starting time and endtime of the motor
unsigned long StartTime;
unsigned long EndTime=120;

unsigned long StartTimer;
unsigned long EndTimer;

// two variable to store the number of times motor is running and second to denote the total times

unsigned int TotalTimes;

// Address variable 
int StateAddress=0;
int TimeAddress=1;

String ON="ON";
String OFF="OFF";

DynamicJsonBuffer jsonBuffer(4000);

void setup() {  
   delay(3000);

  lcd.begin(16, 2);
  lcd.init();
  // Turn on the backlight.
  lcd.backlight();
  // Move the cursor characters to the right and
  // zero characters down (line 1).
   lcd.setCursor(5, 0);
  // Print HELLO to the screen, starting at 5,0.
  lcd.print("WELCOME");
 // Move the cursor to the next line and print
  // WORLD.
  lcd.setCursor(5, 1);
  lcd.print(name);

  delay(1000);
  lcd.clear();

  lcd.setCursor(3,0);
  lcd.print("Intialising");

  lcd.setCursor(5,1);
  lcd.print("Pins");

  // Pin configurations
  
  pinMode(Blue,OUTPUT);
  pinMode(Green,OUTPUT);
  pinMode(MainSwitch,OUTPUT);
  pinMode(relay,OUTPUT);

 // Configuring the serial monitor settings
 
  Serial.begin(115200);
  while(!Serial);
  Serial.println();

  Serial.print("Hello MR.");
  Serial.println(name);
  
  Serial.println();

  Serial.println("Hotspot created by  this ssid: ");
  Serial.println(ssid);

  // Network settings
  
   WiFi.mode(WIFI_AP_STA);
   
   // Wifi connection to connect to the existing Network
    
   Serial.print("Connecting to the SSID: ");
   Serial.println(AWS_ssid);

   delay(1000);
   lcd.clear();
   lcd.setCursor(2,0);
   lcd.print("Connecting..");
   WiFi.begin(AWS_ssid,AWS_password);

   unsigned Wait_1=millis()/1000;
   int flag=0;
   int count=0;
   while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting..");
      lcd.setCursor(5,1);
      lcd.print(count);
      count++;
      if((millis()/1000)>Wait_1+10)
       {
        flag=1;
        Serial.println("Breaking the Loop");
        break; 
       }
     }
     lcd.clear();
     
     if(flag==0)
     {
      lcd.clear();
      lcd.setCursor(3,0);
      lcd.print("Connected!");
     }
     else{
      lcd.clear();
      lcd.setCursor(3,0);
      lcd.print("Unable to");
      lcd.setCursor(1,1);
      lcd.print("Connect to Wifi");
     }
   
    Serial.println();
    Serial.println("Connected to the Wifi");

  // End of the Wifi connection Part

  // Creation of the personal hotspot

   WiFi.softAP(ssid,pass);
               
   server.begin();
   Serial.println("Server started");
   
   Serial.print("IP: ");     
   Serial.println(WiFi.softAPIP());
 
  // Done  Now Turn on the Power Light 
   
   digitalWrite(MainSwitch,HIGH);
   digitalWrite(Green,LOW);

   // Fetching the data from the web server related to the last state of the device
   
   if(flag!=1)
   {
   lcd.clear(); 
   HTTPClient http;
   
   lcd.setCursor(2,0);
   lcd.print("Making GET");
   lcd.setCursor(4,1);
   lcd.print("Request");

   String url="https://x2pgzvtjxj.execute-api.us-west-2.amazonaws.com/Production/motorstates?id=";
   url+=app_id;
   url+="&secret=";
   url+=app_secret;
   url+="&type=";
   url+=type;

   int httpBeginCode = http.begin(url,fingerPrint);
   
   http.addHeader("Content-Type", "application/json");

   int httpRequestCode = http.GET();

   Serial.print("Http Begin Code is: ");
   Serial.println(httpBeginCode);
   
   Serial.print("Http Request Code is: ");
   Serial.println(httpRequestCode);   
    
   if(httpBeginCode)
   {
     unsigned Wait=millis()/1000;
     while(httpRequestCode!=200)
     {
       delay(50);
       Serial.println("Trying Again....");
       httpRequestCode = http.GET(); 
       if((millis()/1000)>Wait+20)
       {
        Serial.println("Breaking the Loop");
        break; 
       }
     }
   }
   else{
    Serial.println("httpBeginCode is -1");
    Serial.println(httpBeginCode);
   }

   if (httpRequestCode > 0) {
                                           //Check the returning code
        lcd.clear();
        lcd.setCursor(2,0);
        lcd.print("GET Success");

       Serial.println("Welcome umesh");
       String payload = http.getString();                     //Get the request response payload
        
       char sms[500];
       strcpy(sms,payload.c_str());
       
       JsonObject& root = jsonBuffer.parseObject(sms);
       Serial.print("The value of the sms");
       Serial.println(sms);
     
       if(root.success())
        {
          Serial.println("Data inside the root object: ");
          root.prettyPrintTo(Serial);
          Serial.println();
 
          String state=root["LastState"];
          Serial.print("The content of the json object is :");
          Serial.println(state);

          if(state=="ON")
          {
            Serial.println("Turning all the thing to its intiak state");
            digitalWrite(Blue,LOW);
            digitalWrite(Green,LOW);
            digitalWrite(MainSwitch,LOW);
            digitalWrite(relay,LOW);
            Serial.println("Reset Done Successfully!");
            delay(1000);
          }
          else if(state=="OFF")
           { 
             Serial.println("Well All the things were OFF already please take care of it");
            }
        }
       else{
          Serial.println("Failed to parse the data");
        } 
      }
    else{
        lcd.clear();
        lcd.setCursor(2,0);
        lcd.print("GET Failed");

        digitalWrite(Green,LOW);
        digitalWrite(relay,LOW);
        digitalWrite(Blue,LOW);
        Serial.println("You are in the else part");
        Serial.println("After the payload statement");
     }
 
    http.end();   //Close connection
  }
   //End of the get request

   EEPROM.begin(512);
   
   int value=EEPROM.read(StateAddress);

   TotalTimes=EEPROM.read(TimeAddress);
   
   if(TotalTimes>0)
   {
    Serial.print("Motor Has Run: ");
    Serial.print(TotalTimes);               // TO bt Printed on the lcd
    Serial.println(" Number of times");
   }
   
   if(value==-1)
  {
    Serial.println("No state is there intially");
  }
  else if(value==0)
  {
    Serial.println("Motor is OFF");
    digitalWrite(Blue,LOW);
  }
  else if(value==1)
  { 
    EEPROM.write(StateAddress,0);
    Serial.println("Motor Was ON, Reseting all to default");
    digitalWrite(Blue,LOW);
  }
  else {
    Serial.println("Nothing Cant be said now....");
  }

  Serial.println("Now going to wait for 1 Minutes Please have a look  at your details");
  delay(1000);
  
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("Starting..");

  StartTime=millis()/1000;
}


void loop()
{ 

  WiFiClient client=server.available();    // Making the server available to client to connect

  if(client)                               // If a client connected to it
    {
       lcd.clear();
       lcd.setCursor(5,0);
       lcd.print("Client");
       lcd.setCursor(3,1);
       lcd.print("Available");

       Serial.println("Client connected to the server");
       digitalWrite(Blue,HIGH);              // Turn on the Blue light
       token=0;                              // intially the value of the token is -1 setting it to 0
       value=0;
       timer=0;
      while(client.connected())               // Running the while loop till client is connected to the server
      { 
        lcd.clear();
        lcd.setCursor(2,0);
        lcd.print("Client State");
        if(client.available())                // if there is something to read from the client use the if loop
          {
           value=0;
           String states=client.readStringUntil('\r');
           states.trim();

           if(states.equals(ON) && token==0) 
            {
             On=1;
             token=1;
             timer=timer+1;
             StartTimer=millis()/1000;                            // Store the current time in this variable
             EEPROM.write(StateAddress,token);                    // Writing the state = 1 in the memory 
             
             digitalWrite(relay,HIGH);                            // Turn On the motor
             digitalWrite(Green,HIGH);
             Serial.println("State ON");
             lcd.setCursor(2,1);
             lcd.print("Motor ON");   
            }

            if(states.equals(ON) && token==1)
            {
              timer=timer+1;
            }

           if (states.equals(OFF) && token==1)
             {
              Off=1;
              token=0;
              timer=0;
              EndTimer=millis()/1000;                                // Store the end Timer value in this variable
              
              EEPROM.write(StateAddress,token);                      // Writing the state = 0 in the memory 
              digitalWrite(relay,LOW);                                // Turn Off the motor
              digitalWrite(Green,LOW);
              Serial.println("State OFF");
              lcd.setCursor(2,1);
              lcd.print("Motor OFF");


              TimeBetON_OFF=(EndTimer-StartTimer)*0.000277778; // in Hours
             
              StartTimer=0;
              EndTimer=0;           
             }

           if(states.equals(ON) && timer>=TimeOut && token==1) 
           {
            timer=0;
            token=0;
            postFlag=1;

            EEPROM.write(StateAddress,token);                          // Writing the state = 0 in the memory 
            digitalWrite(relay,LOW);
            Serial.println("State OFF");
            lcd.setCursor(2,1);
            lcd.print("Motor OFF");                                 // Motor is turned Off if the timeout period exceeded
            digitalWrite(Green,LOW);

            TimeBetON_OFF=(EndTimer-StartTimer)*0.000277778; // in Hours
            StartTimer=0;
            EndTimer=0;
           }

           if(states!=ON && states!=OFF)
           {
            Serial.print("This is the volume inside the tank: ");
            waterLevel=states;                                            // Store the last state of the level in the tank
            Serial.println(states);  
             lcd.setCursor(1,1);
             lcd.print("WaterLevel:");
             lcd.setCursor(12,1);
             lcd.print(states);                                     // Need to print this value on the lcd screen
            delay(100);
           }
           if(On==1 && Off==1)
           {
            On=0;
            Off=0;
            NumberOfTimes=NumberOfTimes+1;                              // Calculating the how many times the motor has already Switched on and off
            FinalState="OFF";
            // checking the level of tank again beore going into deep Sleep
            unsigned long currentTime=millis()/1000;

            /*while(millis()/1000>currentTime+15)
            {
              String states=client.readStringUntil('\r');
              states.trim();
              if(states!=ON && states!=OFF)
              {
                float newValue=(float)states;
                if(newValue>=TankCapacity) 
                 {
                  Indicator=1;
                 }
                 else if(newValue>=(TankCapacity/2) && newValue<TankCapacity){
                  Indicator=2;
                 }
                 else if(newValue<(TankCapacity/2)){
                    Indicator=0;
                 }
              }
            } 
           
            if(Indicator==1)
            {
              Message="Tank Is Full Going To Deep Sleep !";
            }
            else if(Indicator==2)
            {
              Message="Tank Is Half, Going For A short Nap";
            }
            else if(Indicator==0){
              Message="Tank is Empty so I am Awake to look over it";
            }
            */

            //Preparing to make the post request to the server
            /*if(Indicator==1 || Indicator==2 & postFlag=1)
            {*/
            
            HTTPClient newHttp;

            JsonObject& root = jsonBuffer.createObject();

            root["id"]     = app_id;
            root["secret"] = app_secret;
            root["state"]  = FinalState;
            root["level"]  = waterLevel;
            root["time_b"] = TimeBetON_OFF;
            root["time_t"] = NumberOfTimes;
            root["quality"]= Quality;
            root["message"]= Message;
           
            root.prettyPrintTo(Serial);
           
            size_t requestBodySize = root.measureLength() + 1;
            char requestBody[requestBodySize];
           
            root.printTo(requestBody, requestBodySize);
            Serial.print("Lets see the content inside the requestBdy: ");
            Serial.println(requestBody);

            Serial.println();
            Serial.println(".....................................");
            Serial.println();
    
           
            int httpPOSTCode = newHttp.POST(requestBody);

            // Printing responses
            Serial.println("..............................");
            //Serial.println(httpBeginCode);
            Serial.println(httpPOSTCode);
            Serial.println("..............................");
           
            if(httpPOSTCode==200)
            {
              Serial.println("Successfully Post the data");
              postFlag=-1;
            }
            else{
               Serial.println("Failed to POST");
               postFlag=1;
             }
            // Ending http request
            newHttp.end();
           // }

            // End
            EEPROM.write(TimeAddress,NumberOfTimes);                   // Saving the Number of times in memory
           }
          }      // End of the if condition
          else{
            lcd.clear();
            lcd.setCursor(1,0);
            lcd.print("Motor OFF");
            lcd.setCursor(1,1);
            lcd.print(" Not Client"); 
            Serial.println("Client not available");
            value++;
            if(value>WaitingTime)
            {
              digitalWrite(Blue,LOW);
              digitalWrite(relay,LOW);
              digitalWrite(Green,LOW);
              ESP.restart();
            }
          }
       }         // End of the while loop
    }            // End of main if condition
 else
   {
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("Client Not");
    lcd.setCursor(3,1);
    lcd.print("Available");

    digitalWrite(Blue,LOW);                                             // Turn off the internet connection light as the client is disconnected from the server
    Serial.println("You are in the not client connected condition");    // Print this statement on the lcd 
    if(token==1)
      {  Off=1;
         timer=0;
         token=0;
         EEPROM.write(StateAddress,token);                       // Writing the state = 0 in the memory 
         digitalWrite(relay,LOW);                                // Turn off the motor
         digitalWrite(Green,LOW);
         Serial.println("State OFF");
      }

     if(token==-1)
     {
      EEPROM.write(StateAddress,token);
     }

     if(On==1 && Off==1)
     {
      On=0;
      Off=0;
      NumberOfTimes++;
      
      //Preparing to make the post request to the server
            
            HTTPClient newHttp;
            newHttp.addHeader("Content-Type", "application/json");

            JsonObject& root = jsonBuffer.createObject();

            root["id"]     = app_id;
            root["secret"] = app_secret;
            root["state"]  = "OFF";
            root["level"]  = waterLevel;
            root["time_b"] = TimeBetON_OFF;
            root["time_t"] = NumberOfTimes;
            root["quality"]= Quality;
            root["message"]= "Suddenly client get disconnected From the server";
           
            root.prettyPrintTo(Serial);
           
            size_t requestBodySize = root.measureLength() + 1;
            char requestBody[requestBodySize];
           
            root.printTo(requestBody, requestBodySize);
            Serial.print("Lets see the content inside the requestBdy: ");
            Serial.println(requestBody);

            Serial.println();
            Serial.println(".....................................");
            Serial.println();
    
            int httpPOSTCode = newHttp.POST(requestBody);

            // Printing responses
            Serial.println("..............................");
            //Serial.println(httpBeginCode);
            Serial.println(httpPOSTCode);
            Serial.println("..............................");

            // Ending http request
            newHttp.end();

      EEPROM.write(TimeAddress,NumberOfTimes);
     }

     Serial.println("Trying to connect to the Client");

     if((millis()/1000)>t+WaitingTime)            // Waiting for n minutes
     {
       ESP.restart();                            // Restarting the ESP board if no client is connected to the server within the given time interval
     }

     delay(20);
   }  // End of the else part

  }  // End of the Loop Function

