#include <SoftwareSerial.h>

SoftwareSerial wifiSerial(2,3);    // RX, TX of arduino

String ssid="D-Link_DIR-600M";           // Wifi network SSID
String password ="wifi1234";           // Wifi network password
String host ="api.thingspeak.com";  // host server
#define RELAY1 9                   // Light 1
#define RELAY2 10                  // Light 2
#define RELAY3 11                  // Light 3
#define RELAY4 12                  // Fan

boolean DEBUG = true;               //show additional data
int responseTime = 1000;           //communication timeout
int state=0;                       // variable to store state of relay

void setup()
{
  pinMode(RELAY1,OUTPUT);                  //  Pins 9,10,11 and 12 made as output pin for relays 
  pinMode(RELAY2,OUTPUT);
  pinMode(RELAY3,OUTPUT);
  pinMode(RELAY4,OUTPUT);
  
  digitalWrite(RELAY1,HIGH);               //  All relays turned OFF initially
  digitalWrite(RELAY2,HIGH);
  digitalWrite(RELAY3,HIGH);
  digitalWrite(RELAY4,HIGH);
  
  Serial.begin(9600);
  while (!Serial) {
     Serial.println(".");  }           // wait for serial port to connect.  
  Serial.println("Serial started");
    
  wifiSerial.begin(9600);
  while (!wifiSerial) {
      Serial.println(".");    }         // wait for serial port to connect.  
  Serial.println("esp connected");
  
  sendToWifi("AT",responseTime,DEBUG);
  sendToWifi("AT+CWMODE=1",responseTime,DEBUG);                             // configure as Station mode
  sendToWifi("AT+CWJAP=\""+ssid+"\",\""+password+"\"",responseTime,DEBUG);  // connect to wifi
  delay(10000);
  sendToWifi("AT+CIFSR",responseTime,DEBUG);                                // get ip address
  sendToWifi("AT+CIPMUX=0",responseTime,DEBUG);                             // configure for single connections
 
  Serial.println("Wifi connection is running!");
  
}


void loop()
{ 
  state=deviceStatus(1);                                // Check status of device 1
  if(state==1) { digitalWrite(RELAY1,LOW);}             // RELAY1 ON
  if(state==0) { digitalWrite(RELAY1,HIGH);}            // RELAY1 OFF
  
  state=deviceStatus(2);                                // Check status of device 2
  if(state==1)  {digitalWrite(RELAY2,LOW);}             // RELAY2 ON
  if(state==0)  {digitalWrite(RELAY2,HIGH);}            // RELAY2 OFF
  
  state=deviceStatus(3);                                // Check status of device 3
  if(state==1)  {digitalWrite(RELAY3,LOW);}             // RELAY3 ON
  if(state==0)  {digitalWrite(RELAY3,HIGH);}            // RELAY3 OFF
  
  state=deviceStatus(4);                                // Check status of device 4
  if(state==1)  {digitalWrite(RELAY4,LOW);}             // RELAY4 ON
  if(state==0)  {digitalWrite(RELAY4,HIGH);}            // RELAY4 OFF
    
}

int deviceStatus(int r)
{ 
  String getStr="GET /channels/448312/fields/";
  getStr+=r;
  getStr+="/last.txt\r\n\r\n";
  String len="";
  len+=getStr.length();
  
  sendToWifi("AT+CIPSTART=\"TCP\",\""+host+"\",80",responseTime,DEBUG);       // Start TCP connection with api.thingspeak.com at port 80
 
  sendToWifi("AT+CIPSEND="+len,responseTime,DEBUG);                          // gives the String length to be sent  

  String result=sendToWifi(getStr,responseTime,DEBUG);                       // HTTP GET request to get a particular data from thingspeak server
 
  if(find(result,"OK"))
  {    if(find(result,"IPD,1:1"))
           {  Serial.print(r);   Serial.println(" ON");   return 1;   }
       else 
           {  Serial.print(r);   Serial.println(" OFF");  return 0;   }   
  }
  else
     {   Serial.println("ERROR");      return (-1);       }
}

boolean find(String string, String value)                     // function to check if a particular string is a part of 
{                                                             // another string or not
  if(string.indexOf(value)>=0)
    return true;
  return false;
}

String sendToWifi(String command, const int timeout, boolean debug)      // function for sending data to esp8266
{                                                                             
  String response = "";                            
  Serial.println("\nSEND TO WIFI");
  Serial.println(command);
  command+="\r\n\r\n";
  wifiSerial.print(command);               // send command to the esp8266
  long int time = millis();
  while( (time+timeout) > millis())
  {
    while(wifiSerial.available())
    {
    char c = wifiSerial.read();       // read the next character.
    response+=c;
    }  
  }
  Serial.println("\n\nWIFI RESPONSE-----------------------------");
  if(debug)
  {
    Serial.println(response);        // The esp has data so display its output to the serial window
  }
  Serial.println("-------------------------------------------");
  return response;                 //  return the data received from esp8266
}


