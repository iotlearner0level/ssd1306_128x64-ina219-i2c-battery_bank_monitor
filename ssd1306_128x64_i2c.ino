// Libraries
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_INA219.h"
#include "ThingSpeak.h"
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 

#define THINGSPEAKTIMEOUT 120000
#define SERVERTIMEOUT 2000
long servermillis=millis();
long prevMillis=millis();

unsigned long myChannelNumber = 31461;
const char * myWriteAPIKey = "";


const char* ssid = "******";
const char* password = "*******";

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);
WiFiClient  TSclient;
// INA sensor
Adafruit_INA219 ina219;

// OLED screen
#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_RESET);
//Adafruit_SSD1306 display();
#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// Measurement variables
double current_mA;
double power_mW;
double voltage;
double v_2,p_2,p_tot,v_tot;

void setup()   {     
             
  Serial.begin(115200);
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset saved settings
  wifiManager.resetSettings();

  //set custom ip for portal
  //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("AutoConnectAP");
  //or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();


  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  // Init INA219
  ina219.begin();
  ina219.setCalibration_32V_10A();
//  ina219.setCalibration_16V_400mA();

  // Init OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);  

  // Display welcome message
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,26);
  display.println("Ready!");
  display.display();
  delay(1000);
  beginWifi();
  ThingSpeak.begin(TSclient);
    display.println("Starting thingspeak client!");
  display.display();
  delay(1000);

  // LED
  pinMode(D7, OUTPUT);
 delay(1000);
 setupOTA();
}


void loop() {
  yield();

  // LED OFF
  digitalWrite(D7, LOW);

  // Measure
  current_mA = measureCurrent();
  power_mW = measurePower();
  

  // LED ON
  digitalWrite(D7, HIGH);

 
  current_mA = measureCurrent();
  power_mW = measurePower();
  v_2=analogRead(A0)/10.24*25.0;
   // Display data
  v_tot=abs(voltage)+abs(v_2);
  p_tot=current_mA*v_tot;
  p_2=current_mA*v_2;
  displayData(voltage, current_mA, power_mW, v_2);
  ajaxDump(voltage, current_mA, power_mW,v_2,p_2,v_tot,p_tot);
  thingspeakpush((float)voltage,(float) current_mA, (float)power_mW,(float)v_2,(float)p_2,(float)v_tot,(float)p_tot);
  delay(200);
  ArduinoOTA.handle();
}

// Function to measure current
float measureCurrent() {

  // Measure
  double shuntvoltage = ina219.getShuntVoltage_mV();
  double busvoltage = ina219.getBusVoltage_V();
  double current_mA = ina219.getCurrent_mA();
  double loadvoltage = busvoltage + (shuntvoltage / 1000);
  
  Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
  Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
  Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
  Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" A");
  Serial.println("");

  /* If negative, set to zero
  if (current_mA < 0) {
    current_mA = 0.0; 
  }
 */
  return current_mA;
  
}

// Function to measure power
float measurePower() {

  // Measure
  double shuntvoltage = ina219.getShuntVoltage_mV();
  double busvoltage = ina219.getBusVoltage_V();
  double current_mA = ina219.getCurrent_mA();
  double loadvoltage = busvoltage + (shuntvoltage / 1000);
  voltage=loadvoltage;
  
  Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
  Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
  Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
  Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" A");
  Serial.println("");

  /* If negative, set to zero
  if (current_mA < 0) {
    current_mA = 0.0; 
  }
 */
  return current_mA * loadvoltage;
  
}

// Display measurement data
void displayData(float voltage, float current, float power, float batt2) {

  // Clear
  
  display.clearDisplay();  
  display.setCursor(0,0);
  display.setTextSize(1);display.println("");
  // Current
  display.setTextSize(2);

//  display.print("1:");
  display.print(abs(current));
  display.setTextSize(1);
  display.print("A ");
  display.setTextSize(2);
  display.print(abs(voltage)+abs(v_2)*current);
  display.setTextSize(1);
  display.print("W");
  display.setTextSize(2);
  display.println("");


//  display.setTextSize(1);
  // voltages
//  display.print("V1:");
  display.print(voltage);
    display.setTextSize(1);
  display.print("V1 ");

//  display.print("2:");
  display.setTextSize(2);
  display.print(v_2);
  display.setTextSize(1);
  display.print("V2");
  display.setTextSize(2);display.println("");

  // Power
  
/*
  display.print("P: ");
  display.print(power);
  display.println(" W");
*/
 // display.print("Total:");
  display.print(voltage+v_2);
  display.setTextSize(1);
  display.print("V");
  display.setTextSize(1);
   if(current>0)
  display.print(" CHARGING");
  else 
  display.print(" DISCHARGING"); 
  

 display.setTextSize(2);
  display.println("");


 
  display.setTextSize(1);
  int timeleft=(THINGSPEAKTIMEOUT-(millis()-prevMillis))/1000;
  display.print(timeleft);
  display.print(" IP:");
  display.print(WiFi.localIP().toString());
  display.println("");
 
  display.display();
  
}

void beginWifi(){
    // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  // Current
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println("Connecting to...");
  display.println("");
  display.println("");
  display.setTextSize(3);
  display.print(ssid);
  display.display();  


/*  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
*/
  Serial.println("");
  Serial.println("WiFi connected");

  
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
    display.clearDisplay();
    display.setCursor(0,0);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println("Wifi Connected. \nIP");
  display.setTextSize(2);
  display.print(WiFi.localIP());
  display.display(); 
  delay(2000);

}

void readWifi(float voltage, float current, float power){
  
    // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
  
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();
  
  
  client.flush();

  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n <b>Voltage = </b>";
  s += voltage;
  s+="</p><p><b>Current= </b>";
  s+=current;
  s+="</p><p><b> Power= </b>";
  s+=power;
  s += "</p></html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");

  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed

}

void ajaxDump(float voltage, float current, float power, float v2,float p2,float vtot,float ptot){

  String s="<h2>Battery 1:</h2><table><tr><td><b>Voltage </b></td><td id='voltage_b1'>";
  s += voltage;
  s+=" V</td> </tr><tr><td><b>Current </b></td><td id='current' class='switch_txt'>";
  s+=current;
  s+="A</td></tr><tr><td><b> Power </b></td><td id='power_b1' class='switch_txt'>";
  s+=power;
  s+= " W</td></tr> </table><h2>Battery 2</h2><table><tr><td><b>Voltage</b></td><td  id='voltage_b2' class='switch_txt'>";
  s+= v2;
  s+=" V</td></tr><tr><td><b>Power</b></td><td id='power_b2' class='switch_txt'>";
  s+=v2*current;

  s+= " W</td></tr></table><h2>Battery Bank</h2><table><tr><td> <b>Voltage</b></td><td  id='voltage_bb'>";
  s+= abs(voltage)+abs(v2);
  s+=" V</td></tr><tr><td><b>Power</b></td><td  id='power_bb' class='switch_txt'>";
  s+=(abs(voltage)+abs(v2))*abs(current);
  s+=" Watts</td></tr></table>";




   String s2;
//  s2+="<?xml version='1.0' ?>";
//  s2+="<xml>";
  s2="<v_b1>";
  s2+= voltage;
  s2+="</v_b1> <i1>";
  s2+=current;
  s2+="</i1><p_b1>";
  s2+=power;
  s2+= "</p_b1><v_b2>";
  s2+= v2;
  s2+="</v_b2><p_b2>";
  s2+=v2*current;

  s2+= "</p_b2> <v_bb>";
  s2+= abs(voltage)+abs(v2);
  s2+=" </v_bb><p_bb> ";
  s2+=(abs(voltage)+abs(v2))*abs(current);
  s2+=" </p_bb> ";


     String s1;
  s2="{\"v_b1\":";
  s2+= voltage;
  s2+=", \"i1\": ";
  s2+=current;
  s2+=",\"p_b1\":";
  s2+=power;
  s2+= ",\"v_b2\":";
  s2+= v2;
  s2+=",\"p_b2\":";
  s2+=v2*current;
  s2+= ",\"v_bb\":";
  s2+= abs(voltage)+abs(v2);
  s2+=",\"p_bb\":";
  s2+=(abs(voltage)+abs(v2))*abs(current);
  s2+=" } ";
  
  String jsontext="document.getElementById('current').innerHTML=jst1.i1+' A';\n " ;
  jsontext+="document.getElementById('voltage_b1').innerHTML=jst1.v_b1+'V';\n";                        
  jsontext+="document.getElementById('voltage_b2').innerHTML=jst1.v_b2+'V';\n";                        
  jsontext+="document.getElementById('voltage_bb').innerHTML=jst1.v_bb+'V';\n";                        
  jsontext+="document.getElementById('power_b1').innerHTML=jst1.p_b1+' W';\n";                        
  jsontext+="document.getElementById('power_b2').innerHTML=jst1.p_b2+' W';\n";                        
  jsontext+="document.getElementById('power_bb').innerHTML=jst1.p_bb+' W';\n";
   

      // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
 if(millis()-servermillis> SERVERTIMEOUT){

  display.setCursor(0,0);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print("                         ");
  display.setCursor(0,0);
  display.print("Client: ");
  String addy = client.remoteIP().toString();
  display.print(addy);display.print("   ");
  display.display();
    
  // Wait until the client sends some data
  Serial.print("new client:");
  Serial.print(addy);
  while(!client.available()){
    delay(1);
  }
  String HTTP_req = client.readStringUntil('\r');
      if (client) { 
        boolean currentLineIsBlank = true;
        while (client.connected()) {

            if (client.available()) { 
                char c = client.read();
                HTTP_req += c;
                if (c == '\n' && currentLineIsBlank) {
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connection: keep-alive");
                    client.println();
                    if (HTTP_req.indexOf("update") > -1) {
//                      client.print(s1);
                      client.print(s2);
                      Serial.print("Sending JSON text");Serial.println(s2);
//                      server.send(200,"text/xml",s2);
                      
                    } else {
                        client.println("<!DOCTYPE html>");
                        client.println("<html>");
                        client.println("<head>");
                        client.println("<title>Battery Bank Monitor</title>");
                        client.println("<script>");
                        client.println("function getBatteryBank() {");
                        client.println("nocache = \"&nocache=\"+ Math.random() * 1000000;");
                        client.println("var request = new XMLHttpRequest();");
                        client.println("request.onreadystatechange = function getJSONData() {");
                        client.println("if (this.readyState == 4) {");
                        client.println("if (this.status == 200) {");
                        client.println("if (this.responseText != null) {");

                         client.println("document.getElementById('content2').innerHTML = this.responseXML;");

                        client.println("document.getElementById('content1').innerHTML = this.responseText;");
                        client.println("var jst1 = JSON.parse(this.responseText);");
                        client.println("if(jst1!=null){");
                       client.println(jsontext);
                        client.println("}");
                        client.println("document.getElementById('text1').innerText=this.responseText");
                        client.println("document.getElementById('text2').innerHTML=this.responseXML"); 
/*                        
                        client.println("document.getElementById('current').innerHTML=document.getElementById('content1').getElementsByTagName('i1')[0].firstChild.nodeValue;")+" V";
                        client.println("document.getElementById('voltage_b1').innerHTML=document.getElementById('content1').getElementsByTagName('v_b1')[0].firstChild.nodeValue;")+" A";                        
                        client.println("document.getElementById('voltage_b2').innerHTML=document.getElementById('content1').getElementsByTagName('v_b2')[0].firstChild.nodeValue;")+" W";                        
                        client.println("document.getElementById('voltage_bb').innerHTML=document.getElementById('content1').getElementsByTagName('v_bb')[0].firstChild.nodeValue;")+ "V";                        
                        client.println("document.getElementById('power_b1').innerHTML=document.getElementById('content1').getElementsByTagName('p_b1')[0].firstChild.nodeValue;")+ " W";                        
                        client.println("document.getElementById('power_b2').innerHTML=document.getElementById('content1').getElementsByTagName('p_b2')[0].firstChild.nodeValue;")+ " V";                        
                        client.println("document.getElementById('power_bb').innerHTML=document.getElementById('content1').getElementsByTagName('p_bb')[0].firstChild.nodeValue;")+" W";
*/
                        client.println("}}}}");
                        client.println("request.open(\"PUT\", \"update\" + nocache, true);");
                        client.println("request.send(null);");
                        client.println("setTimeout('getBatteryBank()', 3000);");
                        client.println("}");
                        client.println("</script>");
                        client.println("</head>");
                        client.println("<body onload=\"getBatteryBank()\">");
                        client.println("<h1>ESP8266 Battery Bank web server</h1>");
                        client.println("<font color=\"#6a5acd\"><body bgcolor=\"#a0dFfe\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">\n</div>\n<div style=\"clear:both;\"></div><p>");                                               
                        client.println("<h2>Display of Battery Bank Parameters</h2>");
                        client.print(s);
                        client.print("<p id='text1'>This will be text response</p>");
                        client.print("<p id='text2'>This will be text response</p>");
                        client.print("<span id='content1'> This will update Later'</span>");
                        client.print("<span id='content2'> This will update Later'</span>");
                        client.println("<p><a href=\"http://www.esp8266.com\">ESP8266 Support Forum</a></p>");
                        client.println("</body>");
                        client.println("</html>");
                    }
                    Serial.println(HTTP_req);
                    HTTP_req = "";
                    break;
                }
            }
        }
      }}
        delay(1);
        client.stop();
    
}
void thingspeakpush(float v1, float current, float p1,float v2, float p2, float vtot, float ptot){
  

  if (millis()- prevMillis>THINGSPEAKTIMEOUT){


  display.setTextSize(1);
  display.setTextColor(BLACK);
    display.setCursor(0,0);
  display.println("###################################");
  display.println("+++++++++++++++++++++++++++++++++++");
  display.setTextColor(WHITE);

  display.setCursor(0,0);
  display.println("Update thingspeak...   ");
  display.display();
  Serial.println("Updating thingspeak on timeout");

    ThingSpeak.writeField(myChannelNumber, 1, v1, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 2, v2, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 3, current, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 4, p1, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 5, p2, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 6, vtot, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 7, ptot, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 8, current>0?1:-1, myWriteAPIKey);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Thingspeak Updated");
  Serial.println("Thingspeak Updated");
  display.display();
  prevMillis=millis();  
  }

}

void setupOTA(){
    // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("batterybank");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

    ArduinoOTA.onStart([]() {
   display.setCursor(0,0);display.clearDisplay();
   display.setTextSize(3);display.println("UPDATE");display.display();
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");display.clearDisplay();
   display.setTextSize(3);display.println("UPDATE");display.display();
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
  display.setCursor(0,0);
  display.setTextSize(3);display.println("UPDATE");
 
  display.println("");
  Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  display.print( progress / (total / 100));
  display.display();
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  display.clearDisplay();  display.setTextSize(1);
  display.println("Ready\nIP Address:");  display.setTextSize(1.7);
  display.println(WiFi.localIP());
  display.println("");display.display();
  // Current

}

