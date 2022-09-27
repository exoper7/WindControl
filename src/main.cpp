#include <WiFi.h>
#include <ModbusIP_ESP8266.h>
#include <ArduinoJson.h>
#include <TempSensor.h>
#include <voltage.h>
#include <kControl.h>

//ModbusIP object
ModbusIP mb;
//define modbus registers
#define mr_voltage 10
#define mr_current 12
#define mr_rpm 14
#define mr_power 16
#define mr_T1 18
#define mr_T2 20
#define mr_uptime 22

WiFiServer server(80);

// Set your Static IP address
IPAddress local_IP(10, 8, 0, 220);
IPAddress gateway(10, 8, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(1, 1, 1, 1);

//JSON file for API
DynamicJsonDocument _resp(512);

//real data
float _voltage = 22.33;
float rpm = 123.45;
float power = 0.231;
float current = 1.332;

uint loop_n = 0;
bool K1, K2, K3;

//instances for Temperature sensors
tempSensor T1;
tempSensor T2;

//instance for Voltage mesurment
voltage V0;

//instance for relay controls
kControl control;


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  for(uint8_t i;i<10;i++){
    digitalWrite(LED_BUILTIN, HIGH);
    delay(150);
    digitalWrite(LED_BUILTIN, LOW);
    delay(150);
  }

  Serial.begin(115200);
  Serial.println("Begin");
  // Configures static IP address
  WiFi.config(local_IP, primaryDNS, gateway, subnet);

  WiFi.begin("jajko", "Eureka71!s");
  while (WiFi.status() != WL_CONNECTED) {
    delay(150);
  }
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println(WiFi.localIP());
  
  //Config Modbus IP
  mb.server();
  mb.addHreg(10,0,20);

  //start API server
  server.begin();

  T1.adcPin = A0;
  T2.adcPin = A1;
  V0.adcPin = A2;

}

//Splits float value to 2 modbus registers
void HregFloat(uint16_t _reg, float _val){

  union {
  float asFloat;
  uint16_t asInt[2];  
  } v; 

  v.asFloat = _val;

  mb.Hreg(_reg,v.asInt[0]);
  mb.Hreg(_reg+1,v.asInt[1]);

}

//Update JSON file with data
void preperData(void){
  _resp["voltage"] = V0.U;
  _resp["rpm"] = rpm;
  _resp["power"] = V0.P;
  _resp["currnet"] = V0.I;
  _resp["T1"]["T"] = T1.avgTemp;
  _resp["T2"]["T"] = T2.avgTemp;
  _resp["T1"]["R"] = T1.Resistance;
  _resp["T2"]["R"] = T2.Resistance;
  _resp["uptime"] = millis()/1000.000;
  _resp["K1"] = control.K1;
  _resp["K2"] = control.K2;
  _resp["K3"] = control.K3;
  _resp["timeFromLastChange"] = control.millisFromLastChange / 1000.00;
  _resp["alarm"] = control.Alarm;
  _resp["alarmCode"] = control.AlarmCode;
}

//Process HTTP Get request to API
void processGetRequest(void){
  WiFiClient client = server.available();

  if (client)
  {
    Serial.println(("\n\n##################################\nNew client"));
    // an http request ends with a blank line
    bool currentLineIsBlank = true;
    String _GetRequest = "";

    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        _GetRequest = _GetRequest +c;
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank)
        {
          _GetRequest = _GetRequest.substring(4,_GetRequest.indexOf(" HTTP/1.1"));
          Serial.println((_GetRequest));
          Serial.println(("Sending response"));

          client.print(
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: json\r\n"
            "Connection: close\r\n"  // the connection will be closed after completion of the response
            "\r\n");

          if(_GetRequest.equals("/")){
            client.print(("hello"));
          }else if(_GetRequest.equals("/status")){ 
            preperData();
            serializeJson(_resp,client);
          }else{
            client.print("Bad Request\r\n");
          }

          break;
        }

        if (c == '\n')
        {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r')
        {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(10);

    // close the connection:
    client.stop();
    Serial.println(F("Client disconnected"));
  }
}



void loop() {
  //Call once inside loop() - all magic here
  mb.task();
  processGetRequest();
  HregFloat(mr_voltage,V0.U);
  HregFloat(mr_current,V0.I);
  HregFloat(mr_rpm,rpm);
  HregFloat(mr_power,V0.P);
  HregFloat(mr_uptime,millis()/1000.000);
  delay(10);

  V0.process(control.K2, control.K3);
  control.process(V0.U,T1.avgTemp,T2.avgTemp);
  delay(1);
  

  //trigger every 500ms
  if(loop_n>50){
    loop_n = 0;
    HregFloat(mr_T1,T1.GetAvgTemp());
    HregFloat(mr_T2,T2.GetAvgTemp());
  }else{
    loop_n++;
  }
  
}