#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>

const char* ssid = "LUCA IoT 2.4GHz";
const char* password = "luca5555";
const char* mqtt_server = "broker.mqttdashboard.com";

Servo Servo1;  //Servoจ่ายกาแฟ                 D0
Servo Servo2;  //Servoจ่ายชาไทย                D1
Servo Servo3;  //Servoจ่ายชาเขียว                D2
int Relay1 = 14; //กดปุ่มespresso 40ml         D5
int Relay2 = 12; //กดปุ่มlungo 110ml            D6
int Relay3 = 13; //ปั้มดูดของเหลว1 (นม)           D7
int Relay4 = 15; //ปั้มดูดของเหลว2 (ไซรัป)       D8

String strs[5];
int StringCount = 0;
String msgr;
String messagemachine;
String msgclear;

int OrderId = 0;
int Type = 0;
int Water = 0;
int Milk;
int Syrup;

int Tempid1 = 0;
int Tempid2 = 0;

int Step1;
int Step2;
int Stepwater;
int Stepmilk;
int Stepsyrup;
int Tempdelaymachine = 60000;
unsigned long time_1 = 0;
unsigned long time_2 = 0;



WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String state;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    state += (char)payload[i];
  }
  Serial.println(); 
  if(state == "CLEAR " || state == "SCLEAR "){
  Serial.println("Clear");
          if(state == "CLEAR "){
             digitalWrite(Relay1,LOW);
             delay(1000);
             digitalWrite(Relay1,HIGH);
             delay(500);
             if (!client.connected()) {
             reconnect();
             }
             client.loop();
             Serial.println("DONE MQTTsent to Clearmachine");
             delay(500);
             msgclear = String("SCLEAR")+ String(' ');
             client.publish("OMC/Typewater", msgclear.c_str());
             Serial.println("SClear");
             }else{
              Serial.println("DDDDDDDDD");
              }
  }else if(length > 7){
  msgr = state;
  Serial.println("Server");
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      
      // ... and resubscribe
      client.subscribe("OMC/Typewater");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void Readvalueserver() {
  StringCount = 0;
  Serial.println("Readvalueserver");
  while (msgr.length() > 0) {
    int index = msgr.indexOf(' ');
    if (index == -1) { // No space found
      strs[StringCount++] = msgr;
      break;
    } else {
      strs[StringCount++] = msgr.substring(0, index);
      msgr = msgr.substring(index + 1);
    }
  }

  StringCount = 0;
/*
     for (int i = 0; i < StringCount; i++)
         {
           Serial.print(i);
           Serial.print(": \"");
           Serial.print(strs[i]);
           Serial.println("\"");
         }

  */

  OrderId = strs[0].toInt();
  Type = strs[1].toInt();
  Water = strs[2].toInt();
  Milk = strs[3].toInt();
  Syrup = strs[4].toInt();


  Serial.println("----------------------");
  Serial.println(OrderId);
  Serial.println(Type);
  Serial.println(Water);
  Serial.println(Milk);
  Serial.println(Syrup);
  Serial.println("----------------------");
  delay(1000);
  
}




void setup() {
    Servo1.attach(16); //D0
    Servo2.attach(5); //D1
    Servo3.attach(4); //D2
    pinMode(Relay1, OUTPUT);//D5
    pinMode(Relay2, OUTPUT);//D6
    pinMode(Relay3, OUTPUT);//D7
    pinMode(Relay4, OUTPUT);//D8

    Serial.begin(115200);
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

    Servo1.write(0); // สั่งให้ Servo หมุนไปองศาที่ 0
    delay(500);
    Servo2.write(0); // สั่งให้ Servo หมุนไปองศาที่ 0
    delay(500);
    Servo3.write(0); // สั่งให้ Servo หมุนไปองศาที่ 0
    delay(500); 

    digitalWrite(Relay1, HIGH);
    delay(500); 
    digitalWrite(Relay2, HIGH);
    delay(500); 
    digitalWrite(Relay3, HIGH);
    delay(500); 
    digitalWrite(Relay4, HIGH);
    delay(500); 
    Step1 = 0 ;

}

void loop() {

  if (!client.connected()) {
    reconnect();
    }
   client.loop();
   Readvalueserver();
   Step2 = 0;
   Stepwater = 0;
   Stepmilk = 0;
   Stepsyrup = 0;

   Serial.println("Tempid1 =");
   Serial.println(Tempid1);
   Serial.println("Tempid2  =");
   Serial.println(Tempid2);

   delay(1000);
   
   Serial.println("main loop");
   
   while(Step1 == 0 && OrderId != Tempid1 && Type != 0){
      Serial.println("Step 1");
      if(Type == 1){ 
      Servo1.write(90); 
      delay(1000); 
      Servo1.write(0); // สั่งให้ Servo หมุนไปองศาที่ 0
      delay(1000);// หน่วงเวลา 1000ms
      Tempid1 = OrderId;
      Serial.println("DONE Step 1");
      Step1 = 1; 
      
      }
      else if(Type == 2){
      Servo2.write(90); 
      delay(1000); 
      Servo2.write(0); // สั่งให้ Servo หมุนไปองศาที่ 0
      delay(1000); // หน่วงเวลา 1000ms
      Tempid1 = OrderId;
      Step1 = 1; 
      Serial.println("DONE Step 1");
      }
      else if(Type == 3){
      Servo3.write(90); 
      delay(1000); 
      Servo3.write(0); // สั่งให้ Servo หมุนไปองศาที่ 0
      delay(1000); // หน่วงเวลา 1000ms
      Tempid1 = OrderId;
      Step1 = 1; 
      Serial.println("DONE Step 1");
      }
   }

   while(Step2 == 0 && OrderId != Tempid2 && Water != 0){
       Serial.println("Step 2");
      while(Stepwater == 0){
          Serial.println("Stepwater");
          if(Water == 1){
             digitalWrite(Relay1,LOW);
             delay(2000);
             digitalWrite(Relay1,HIGH);
             Stepwater = 1;
             Serial.println("DONE Stepwater");
             break;
          }else if(Water == 2){
             digitalWrite(Relay2,LOW);
             delay(2000);
             digitalWrite(Relay2,HIGH);
             Stepwater = 1;
             Serial.println("DONE Stepwater");
             break;
          }
       }
       while(Stepmilk == 0){
          Serial.println("Stepmilk");
          digitalWrite(Relay3,LOW);
          delay(Milk);
          digitalWrite(Relay3,HIGH);
          Serial.println("DONE Stepmilk");
          Stepmilk = 1;
          
       }
       while(Stepsyrup == 0){
          Serial.println("Stepsyrup");
          digitalWrite(Relay4,LOW);
          delay(Syrup);
          digitalWrite(Relay4,HIGH);
          delay(1000);
          Serial.println("DONE Stepsyrup");
          Stepsyrup = 1;
         
       }
       if(Stepmilk == 1 && Stepsyrup ==1){
         Tempdelaymachine = Tempdelaymachine - (Milk + Syrup);
         if(Tempdelaymachine <= 0){
         Tempdelaymachine = 1000;
         }
        delay(Tempdelaymachine);
        delay(500);
        if (!client.connected()) {
             reconnect();
        }
        client.loop();
        Serial.println("DONE MQTTsent to Machine");
        delay(500);
        messagemachine = String("SUCCESS")+ String(' ');
              client.publish("OMC/Machine", messagemachine.c_str());
        Tempid2 = OrderId;
        Step1 = 0;
        Step2 = 1;
        }
       
   }
   

  

}
