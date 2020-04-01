#include <ESP8266WiFi.h>
#include <NexaCtrl_ESP.h>
#include <Ticker.h>
#define TX_PIN 4
#define LED_PIN 2

Ticker secondTick;

volatile int watchdogCount = 0;
String s;
const bool useAmp = false;

void ISRwatchdog()
{
  watchdogCount ++;
  if(watchdogCount == 30)
  {
    Serial.println("Watchdog timeout");
    ESP.reset();
  }
}

int AMP_PIN = 5;

const char* ssid = "ssid";
const char* password = "password";
String readString, newString, newString2;

NexaCtrl nexaCtrl(TX_PIN);

WiFiServer server(80);

void setup() {
  secondTick.attach(1, ISRwatchdog);
  pinMode(AMP_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(AMP_PIN, HIGH);
  digitalWrite(LED_PIN, HIGH);
  Serial.begin(115200);
  delay(10);
  WiFi.hostname("Esp Nexa Transmitter");
  // Connect to WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() {
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  watchdogCount = 0;
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  // Wait until the client sends some data
  while(!client.available()){
    delay(1);
  }
  
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  //Serial.println(req);
  //Serial.println(req.length());
  client.flush();
  
  s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n ";
  
  if(req.length() > 25)
  {
    digitalWrite(LED_PIN, LOW);
    ParseString(req);
  }
  digitalWrite(LED_PIN, HIGH);
  client.flush();

  s += "</html>\n";
  client.print(s);
  delay(1);
  //Serial.println("Client disonnected");
}

void ParseString(String Incoming)
{
        int pos1 = Incoming.indexOf('/');
        int pos2 = Incoming.indexOf('\n');
        newString = Incoming.substring(pos1+1, pos2);
        //int pos21 = newString.indexOf(' ');
        int pos22 = newString.indexOf('H');
        newString2 = newString.substring(0, pos22);
        newString2.replace(" ", "");
        Serial.println("New connection:");
        Serial.println(newString2);
        int Index2 = newString2.indexOf(',', 0);
        int Index3 = newString2.indexOf(',', Index2+1);
        int Index4 = newString2.indexOf(',', Index3+1);
        int Index5 = newString2.indexOf(',', Index4+1);
        int Index6 = newString2.indexOf(',', Index5+1);
        int Index7 = newString2.indexOf(',', Index6+1);
        String firstValue = newString2.substring(0, Index2);
        String secondValue = newString2.substring(Index2+1, Index3);  
        String thirdValue = newString2.substring(Index3+1, Index4); 
        String fourthValue = newString2.substring(Index4+1, Index5);
        String fifthValue = newString2.substring(Index5+1, Index6); 
        String Mode;
        unsigned int Unit;
        unsigned long Address;
        int Value;
        int txRepeat;
        Mode = firstValue;
        Unit = secondValue.toInt();
        Address = thirdValue.toInt();
        Value = fourthValue.toInt();
        txRepeat = fifthValue.toInt();
        
        s += "Transmitting:<br />";
        s += "Mode:" + firstValue;
        s += "<br />Unit:" + secondValue;
        s += "<br />Address" + thirdValue;
        s += "<br />Dim Value:" + fourthValue;
        s += "<br />Transmit repeated:" + fifthValue;
                                              //                           Url command order is <Mode>,<Button/Unit(0-15)>,<Controller code>,<Dim level>,<Transmit repeat times>
        if(Mode == "On")                      //                           Example On,0,10646597,0,4 Or Off,0,10646597,0,4 When dimming Dim,1,10646597,7,4 Or DimOff,1,10646597,0,4 Dimming level is 0-15
        {                                     //                           If you want to teach new switch or dimmer use (Teach,"0-15","8 number code",0) = (Teach,0,12345678,0)
          if(useAmp)                          //                           Its better to use random 8 digit number as code for controller address because if you use 12345678
          {                                   //                           there is a chance that someone else has the same code
            digitalWrite(AMP_PIN, LOW);
            delay(100);
          }
          SendNexaOn(Unit,Address,txRepeat);
          if(useAmp)
          {
            delay(100);
            digitalWrite(AMP_PIN, HIGH);
          }
        }
        if(Mode == "Off")
        {
          if(useAmp)
          {
            digitalWrite(AMP_PIN, LOW);
            delay(100);
          }
          SendNexaOff(Unit,Address,txRepeat);
          if(useAmp)
          {
            delay(100);
            digitalWrite(AMP_PIN, HIGH);
          }
        }
        if(Mode == "Teach")
        {
          if(useAmp)
          {
            digitalWrite(AMP_PIN, LOW);
            delay(100);
          }
          TeachNexa(Unit,Address);
          if(useAmp)
          {
            delay(100);
            digitalWrite(AMP_PIN, HIGH);
          }
        }
        if(Mode == "Dim")
        {
          if(useAmp)
          {
            digitalWrite(AMP_PIN, LOW);
            delay(100);
          }
          SendNexaDim(Value, Unit, Address, txRepeat);
          if(useAmp)
          {
            delay(100);
            digitalWrite(AMP_PIN, HIGH);
          }
        } 
        if(Mode == "DimOn")
        {
          if(useAmp)
          {
            digitalWrite(AMP_PIN, LOW);
            delay(100);
          }
          SendNexaDim(Value, Unit, Address, txRepeat);
          if(useAmp)
          {
            delay(100);
            digitalWrite(AMP_PIN, HIGH);
          }
        } 
        if(Mode == "DimOff")
        {
          if(useAmp)
          {
            digitalWrite(AMP_PIN, LOW);
            delay(100);
          }
          SendNexaOff(Unit,Address, txRepeat);
          if(useAmp)
          {
            delay(100);
            digitalWrite(AMP_PIN, HIGH);
          }
        }
}
void SendNexaOn(unsigned int Unit, unsigned long Address, unsigned int txTimes)
{
  for (int x = 0; x < txTimes; x++)
  {
    nexaCtrl.DeviceOn(Address,Unit);
    delay(80);
  }
}
void SendNexaOff(unsigned int Unit, unsigned long Address, unsigned int txTimes)
{
  for (int x = 0; x < txTimes; x++)
  {
    nexaCtrl.DeviceOff(Address,Unit);
    delay(80);
  }
}

void SendNexaDim(int Value, int Unit, unsigned long Address, unsigned int txTimes)
{
  for (int x = 0; x < txTimes; x++)
  {
    nexaCtrl.DeviceDim(Address, Unit, Value);
    delay(80);
  }
}

void TeachNexa(unsigned int Unit, unsigned long Address)
{
  nexaCtrl.DeviceOn(Address,Unit);
  delay(200);
  nexaCtrl.DeviceOn(Address,Unit);
  delay(200);
  nexaCtrl.DeviceOn(Address,Unit);
  delay(200);
  nexaCtrl.DeviceOn(Address,Unit);
  delay(200);
  nexaCtrl.DeviceOn(Address,Unit);
  delay(200);
  nexaCtrl.DeviceOn(Address,Unit);
  delay(200);
  nexaCtrl.DeviceOn(Address,Unit);
  delay(200);
  nexaCtrl.DeviceOn(Address,Unit);
  delay(200);
  nexaCtrl.DeviceOn(Address,Unit);
  delay(200);
  nexaCtrl.DeviceOn(Address,Unit);
  delay(200);
}
