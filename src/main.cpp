#include <WiFi.h>
//#include <ModbusIP_ESP8266.h>
#include <ArduinoJson.h>


#include <TempSensor.h>
#include <voltage.h>
#include <kControl.h>
#include <RPMs.h>
#include <fanControl.h>
#include <energy.h>

//#define APIdebug true

//#define MasterNode


//ModbusIP object
//ModbusIP mb;
//define modbus registers
#define mr_voltage 10
#define mr_current 12
#define mr_rpm 14
#define mr_power 16
#define mr_T1 1
#define mr_T2 20
#define mr_uptime 22

#define RPMpin 15
#define K1pin 4
#define K2pin 3
#define K3pin 2
#define AlarmLED 7
#define D8_LED 6
#define FanPin 1


WiFiServer server(80);

// Set your Static IP address

#ifdef MasterNode
  IPAddress local_IP(10, 217, 60, 120);
#else
  IPAddress local_IP(10, 217, 60, 121);
#endif
IPAddress gateway(10, 217, 0, 10);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(1, 1, 1, 1);

//JSON file for API
DynamicJsonDocument _resp(1024);

//uint loop_n = 0;

u64_t lastSubLoop = 0;

//instances for Temperature sensors
tempSensor T1;
tempSensor T2;
tempSensor MCUtemp;


//instance for Voltage mesurment
voltage V0;

//instance for relay controls
kControl control;

//instance for RPM reading
RPMs _rpm;

//RPM mesurment callback
void RPMcallback(void){
  _rpm.processRPM();
}

fanControl fan;

energy E0;


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RPMpin,INPUT);
  pinMode(K1pin,OUTPUT_12MA);
  pinMode(K2pin,OUTPUT_12MA);
  pinMode(K3pin,OUTPUT_12MA);
  pinMode(AlarmLED,OUTPUT_12MA);
  pinMode(D8_LED,OUTPUT_12MA);

  digitalWrite(AlarmLED,HIGH);

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

  WiFi.begin("APA_GROUP", "apa$$wifi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(150);
  }
  
  Serial.println(WiFi.localIP());
  
  //Config Modbus IP
  //mb.server();
  //mb.addHreg(10,0,20);

  //start API server
  server.begin();

  //Assign adc pins
  T1.adcPin = A2;
  T2.adcPin = A1;
  V0.adcPin = A0;

  V0.setOffset();

  E0.begin();


  //Attach interrupt for rpm mesurment
  attachInterrupt(RPMpin,RPMcallback,RISING);

  //set pwm for fan
  analogWriteFreq(50000);

  //some LED's
  digitalWrite(AlarmLED, LOW);
  digitalWrite(LED_BUILTIN, HIGH);
  //digitalWrite(D8_LED, HIGH);
}

/*
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
*/

String SetFanSpeed(String _strData){
  String _strSpeed = _strData.substring(8);
  fan.Speed = _strSpeed.toInt();
  return (String)fan.Speed;
}

//Update JSON file with data
void preperData(void){
  _resp["voltage"] = V0.U;
  _resp["speed"]["rpm"] = _rpm.RPM;
  _resp["speed"]["f"] = _rpm.Hz;
  _resp["speed"]["mean_f"] = _rpm.meanHz;
  _resp["power"] = V0.P;
  _resp["power_avg"] = V0.Pavg;
  _resp["estimate_power"] = V0.P * 10.0;
  _resp["currnet"] = V0.I;
  _resp["energy"] = E0.E;
  _resp["temps"]["T1"]["T"] = T1.avgTemp;
  _resp["temps"]["T2"]["T"] = T2.avgTemp;
  _resp["temps"]["T1"]["R"] = T1.Resistance;
  _resp["temps"]["T2"]["R"] = T2.Resistance;
  _resp["temps"]["MCU"] = MCUtemp.avgTemp;
  _resp["fanPower"] = fan.Speed;
  _resp["uptime"] = millis()/1000.000;
  _resp["relay"]["K1"] = control.K1;
  _resp["relay"]["K2"] = control.K2;
  _resp["relay"]["K3"] = control.K3;
  _resp["relay"]["currState"] = control.currState;
  _resp["timeFromLastChange"] = control.millisFromLastChange / 1000.00;
  _resp["alarm"] = control.Alarm;
  _resp["alarmCode"] = control.AlarmCode;
  _resp["volt_offset"] = V0.offset;
}

//Process HTTP Get request to API
void processGetRequest(void){
  WiFiClient client = server.available();

  if (client)
  {
    #ifdef APIdebug
      Serial.println(("\n\n##################################\nNew client"));
    #endif
    // an http request ends with a blank line
    bool currentLineIsBlank = true;
    String _GetRequest = "";

    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        _GetRequest = _GetRequest +c;

        #ifdef APIdebug
          Serial.write(c);
        #endif
        
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank)
        {
          _GetRequest = _GetRequest.substring(4,_GetRequest.indexOf(" HTTP/1.1"));

          #ifdef APIdebug
            Serial.println((_GetRequest));
            Serial.println(("Sending response"));
          #endif
          
          client.print(
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: json\r\n"
            "Connection: close\r\n"  // the connection will be closed after completion of the response
            "\r\n");

          if(_GetRequest.equals("/")){
            client.print(("hello, I am a Wind turbine controller more info at myip/status"));
          }else if(_GetRequest.equals("/status")){ 
            preperData();
            serializeJson(_resp,client);
          }else if(_GetRequest.startsWith("/setFan/")){ 
            client.println(_GetRequest.substring(8));
            client.print(SetFanSpeed(_GetRequest));
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


    #ifdef APIdebug
     Serial.println(F("Client disconnected"));
    #endif
  }
}



void loop() {
  //modbusTCP task
  //mb.task();

  //API handler
  processGetRequest();

  /*
  //modbus values here
  HregFloat(mr_voltage,V0.U);
  HregFloat(mr_current,V0.I);
  HregFloat(mr_rpm,_rpm.RPM);
  HregFloat(mr_power,V0.P);
  HregFloat(mr_uptime,millis()/1000.000);
  delay(10);
  */
  //mesure voltage 
  V0.process(control.currState);
  V0.calculateAvgPower();

  //relays
  control.process(V0.U,T1.avgTemp,T2.avgTemp);

  digitalWrite(K1pin,control.K1);
  digitalWrite(K2pin,control.K2);
  digitalWrite(K3pin,control.K3);
  digitalWrite(AlarmLED,control.Alarm);
  delay(10);
  
  //trigger every 500ms
  if(millis()>(lastSubLoop+500)){
    lastSubLoop = millis();
    /*
    //get temperature readings
    HregFloat(mr_T1,T1.GetAvgTemp());
    HregFloat(mr_T2,T2.GetAvgTemp());
    */
    T1.GetAvgTemp();
    T2.GetAvgTemp();
    MCUtemp.avgTemp = MCUtemp.calculateAvgTemp(analogReadTemp(),8);
    //fan.Speeede(T1.avgTemp,T2.avgTemp);
    analogWrite(FanPin,fan.getPwmSpeed());

    // zero rpm handler
    _rpm.update();
    E0.calculateEnergy(V0.Pavg);
  }
}