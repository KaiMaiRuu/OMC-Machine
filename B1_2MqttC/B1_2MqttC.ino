#include <ESP8266WiFi.h>
#include <PubSubClient.h>


const char* ssid = "LUCA IoT 2.4GHz";
const char* password = "luca5555";
const char* mqtt_server = "broker.mqttdashboard.com";

int Motor1_R = 4;///ฝาเครื่อง                   D2
int Motor1_L = 14;//ฝาเครื่อง                   D5
int Motor2_R = 12;//ปล่อยแก้ว                   D6
int Motor2_L = 15;//ปล่อยแก้ว                   D8
int Photo1 = 2; //เช็คแก้ว                      D4
int Photo2 = 13; //เช็คแคปซูล                   D7
int Limitopen = 16; //limitเช็คฝาเครื่องเปิดสุด      D0
int Limitclose = 5; // limitเช็คฝาเครื่องปิดสุด     D1

int Statephoto1glass = 0; //สถานะ Photo1
int Statephoto2capsule = 0; //สถานะ Photo2 เจอ 0 ไม่เจอ 1
int Statelimitopen = 0; //สถานะlimitเปิด  กด 1 ไม่กด 0
int Statelimitclose = 0; //สถานะlimitปิด





String strs[7];  //รับ order
String strstock[2]; //รับสถานะจากstock
String strmilksyrup[1];//รับจากbordคุมจ่าย
int StringCount = 0;
String msgr; //เก็บข้อมูลรับorder
String msgstock; //เก็บสถานะเครื่อง stockว่างไหม
String msgmilksyrup; //เก็บข้อความที่มาจากบอร์ดคุมไซรัปว่า success ทำเสร็จแล้ว
String messagemachine;


int OrderId;
int Type;
int Water;
int Milk;
int Syrup;
int CupAmount;
int IsStocking;

int Tempid = 0;//เก็บidเก่าเทียบ
String Statusmilksyrup; //จ่ายนำไซรัปเสร็ = SUCCESSจ
String Statuscheck;// พร้อมให้เครื่องทำต่อไหมคือลิฟต์เข้าที่ = READY
int Statusstock;// stockว่างไหม 0 ไม่ว่าง 1 ว่าง

String messagewater; //ข้อความส่งไปwater
String messagestock; //ข้อความส่งไปstock
String messageserver; //ข้อความส่งไปserver
String messageupdatestatus;

int Step1;         //สถานะสั่งทำงาน ไม่เสร็จ = 0 เสร็จ= 1
int Stepglass;     //สถานะปล่อยแก้ว ไม่เสร็จ = 0 เสร็จ= 1
int Clear;   //สถานะเปิดปิดฝาเครื่องเคลียแคปซูลก่อนส่ง Clear ล้างเครื่อง  
int Stepclear;


int Checkcap;      //สถานะphotoเจอแคปซูล
int Stepclosemachine; //สถานะปิดฝาเครื่อง ไม่เสร็จ = 0 เสร็จ= 1
int Stepopenmachine; //สถานะเปิดฝาเครื่อง ไม่เสร็จ = 0 เสร็จ= 1
int Countcup; //ทำไปแล้วกี่แก้ว
int Step2; //ให้ลูกค้ารับแก้วแรกในกรณีสั่งไม่stockหลายแก้ว ยังไม่รับ = 0 รับแล้ว= 1
int Step3; //สั่งstock แบบบหลายแก้วไม่เสร็จ = 0 เสร็จ= 1
int Step4; //ส่งค่าไปบอร์ดจ่ายแคปซูล นม ไซ
int Step5; //รอเจอแคปซูล




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
 /*
 if(topic == "OMC/Server1"){
  Serial.println("Server1");
  }else if(topic == "OMC/Server2"){
  Serial.println("Server2");
  }else if(topic == "OMC/Server3"){
  Serial.println("Server3");
  }
 */
 
  if(state == "SUCCESS " || state == "UNSUCCESS "){        //รับจาก board3 ว่าจ่ายนมไซรัปเสร็จแล้ว
     msgmilksyrup = state;
     Serial.println("Server1");
  }else if(length > 12){                                  //รับ order จาก mqtt
     msgr = state;
     Serial.println("Server3");
  }else if(state == "READY 1 " || "READY 0 " || "NOTREADY 1 " || "NOTREADY 0 "){        //รับสถานะจากstock
     msgstock = state;
     Serial.println("Server2");
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
      client.subscribe("OMC/Machine");
      
      
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void Machineopen() {                        
  Statelimitopen = digitalRead(Limitopen);
  Serial.println("Machineopen");
  Stepopenmachine = 0;
  
  while(Stepopenmachine == 0){
    Statelimitopen = digitalRead(Limitopen);
  if (Statelimitopen == 0) {
    analogWrite(Motor1_R, 0);   //เช็ค
    analogWrite(Motor1_L, 150);                   //สั่งเปิดฝาเครื่อง
    Serial.println("do open");
  } else if(Statelimitopen == 1) {
    analogWrite(Motor1_R, 0);
    analogWrite(Motor1_L, 0);
    Serial.println("do stopopen");
    Stepopenmachine = 1;
  }
  }
}

void Machineclear(){
  Statelimitopen = digitalRead(Limitopen);
  while(Statelimitopen == 0){
  Statelimitopen = digitalRead(Limitopen);
  Machineopen(); 
  }
  Stepclear = 0;
  while(Stepclear == 0){
    delay(2000);
    Statelimitclose = digitalRead(Limitclose);
    if (Statelimitclose == 0){
    Serial.println("do close");
    analogWrite(Motor1_R, 150);   //เช็ค
    analogWrite(Motor1_L, 0);
  } else if (Statelimitclose == 1) {
    Serial.println("do stopclose");
    analogWrite(Motor1_R, 0);
    analogWrite(Motor1_L, 0);
    delay(500);
    if (!client.connected()) {
    reconnect();
    }
    client.loop();
    delay(500);
     messagewater =String("CLEAR") + String(' ');
        client.publish("OMC/Typewater", messagewater.c_str());
    delay(30000);
    Clear = 1;
    Stepclear = 1;
    }
  }
  }

void Glass() {
  while(Stepglass == 0){
  Serial.println("Glass");
  Serial.println("do glass");
  analogWrite(Motor2_R, 0);
  analogWrite(Motor2_L, 150);         //ปล่อยแก้ว
  delay(3050);
  Serial.println("do stopglass");
  analogWrite(Motor2_R, 150);
  analogWrite(Motor2_L, 0);
  delay(3050);
  analogWrite(Motor2_R, 0);
  analogWrite(Motor2_L, 0);
  delay(4000);
  Statephoto1glass = digitalRead(Photo1);
  if(Statephoto1glass == 0)
  Stepglass = 1;
  }
}

void Machineclose() {
  Serial.println("Machineclose-------------------------------------------");
while(Stepclosemachine == 0){
  Statelimitclose = digitalRead(Limitclose);
  if (Statelimitclose == 0) {
    Serial.println("do close");
    analogWrite(Motor1_R, 150);   //เช็ค
    analogWrite(Motor1_L, 0);
  } else if (Statelimitclose == 1) {
    Serial.println("Machineclose succees  sentmqtt push botton");
    Serial.println("do stopclose");
    analogWrite(Motor1_R, 0);
    analogWrite(Motor1_L, 0);
    delay(500);
    if (!client.connected()) {
    reconnect();
  }
  client.loop();
  delay(500);
  if(Countcup == 0){
  delay(500);
    messagewater =String(OrderId) + String(' ') + 
                  String(Type) + String(' ') +
                  String(Water) + String(' ') +
                  String(Milk) + String(' ') +
                  String(Syrup) + String(' ');
        client.publish("OMC/Typewater", messagewater.c_str());
    
  }else if(Countcup == 1){
    delay(500);
    messagewater =String(0) + String(' ') + 
                  String(Type) + String(' ') +
                  String(Water) + String(' ') +
                  String(Milk) + String(' ') +
                  String(Syrup) + String(' ');
        client.publish("OMC/Typewater", messagewater.c_str());
    }
  Stepclosemachine = 1;
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
  CupAmount = strs[5].toInt();
  IsStocking = strs[6].toInt();
/*
  Serial.println("Statuscheck = ");
  Serial.println(Statuscheck);
  Serial.println("Statusstock = ");
  Serial.println(Statusstock);
  Serial.println("Statusmilksyrup = ");
  Serial.println(Statusmilksyrup); */
  Serial.println("----------------------");
  Serial.println(OrderId);
  Serial.println(Type);
  Serial.println(Water);
  Serial.println(Milk);
  Serial.println(Syrup);
  Serial.println(CupAmount);
  Serial.println(IsStocking);
  Serial.println("----------------------");
  delay(1000);
  
}

void Readstatusstock(){
  StringCount = 0;
  while (msgstock.length() > 0) {
    int index = msgstock.indexOf(' ');
    if (index == -1) { // No space found
      strstock[StringCount++] = msgstock;
      break;
    } else {
      strstock[StringCount++] = msgstock.substring(0, index);
      msgstock = msgstock.substring(index + 1);
    }
  }

  StringCount = 0;
  

  Statuscheck = strstock[0];               //READY NOTDRADY
  Statusstock = strstock[1].toInt();       //0 ไม่ว่าง 1 ว่าง
  Serial.println("Statuscheck = ");
  Serial.println(Statuscheck);
  Serial.println("Statusstock = ");
  Serial.println(Statusstock);

  delay(1000);
  

}
void Readstatusmilksyrup(){
  StringCount = 0;
  while (msgmilksyrup.length() > 0) {
    int index = msgmilksyrup.indexOf(' ');
    if (index == -1) { // No space found
      strmilksyrup[StringCount++] = msgmilksyrup;
      break;
    } else {
      strmilksyrup[StringCount++] = msgmilksyrup.substring(0, index);
      msgmilksyrup = msgmilksyrup.substring(index + 1);
    }
  }

  StringCount = 0;
  

  Statusmilksyrup = strmilksyrup[0];
  
  Serial.println("Statusmilksyrup = ");
  Serial.println(Statusmilksyrup);
  delay(1000);
}

void Errorcapsule(){
  Serial.println("Error capsule");
  Statephoto2capsule = digitalRead(Photo2);
        while(Statephoto2capsule == 0){
          Statephoto2capsule = digitalRead(Photo2);
          Serial.println("Remove capsule");
          if(Statephoto2capsule == 1){
          delay(500);
            if (!client.connected()) {
                reconnect();
            }
          client.loop();
        delay(500);
        messagewater = String(OrderId) + String(' ') + 
                  String(Type) + String(' ') +
                  String('0') + String(' ') +
                  String('0') + String(' ') +
                  String('0') + String(' ');
        client.publish("OMC/Typewater", messagewater.c_str());
        break;
          }
        }
  }

void Successcapsule(){
  Serial.println("DONE capsule");
        Step5 = 1;
        Checkcap = 1;
  }

void setup() {
  pinMode(Motor1_R, OUTPUT);//D2
  pinMode(Motor1_L, OUTPUT);//D5
  pinMode(Motor2_R, OUTPUT);//D6
  pinMode(Motor2_L, OUTPUT);//D8
  pinMode(Photo1, INPUT); //D4
  pinMode(Photo2, INPUT); //D7
  pinMode(Limitopen , INPUT_PULLUP); //D0
  pinMode(Limitclose , INPUT_PULLUP); //D1

  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  Statephoto1glass = digitalRead(Photo1);
  Statephoto2capsule = digitalRead(Photo2);
  Statelimitopen = digitalRead(Limitopen);
  Statelimitclose = digitalRead(Limitclose);

  Serial.println("setup");


  while (Statephoto1glass == 0) {                              //เช็คแก้วเป็น 0 เจอ วนloop
    Statephoto1glass = digitalRead(Photo1);
    Serial.println("Remove glass");
    delay(3000);
  }

  while (Statelimitopen == 0) {                         //ถ้าไม่เปิดเป็น0 สั่งเปิดฝาเครื่องจนถึงlimitเปิด
    delay(5000);
    Serial.println("setup checklimitopen");
    Statelimitopen = digitalRead(Limitopen);
    Machineopen();
  }

  while (Statephoto2capsule == 0) { //หากเจอแคปซูลค้าง
    Statephoto2capsule = digitalRead(Photo2);
    Serial.println("Remove Capsule");
    delay(3000);
  }

}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  Readstatusstock();
  Readvalueserver();
  Clear = 0;
  Step1 = 0;
  Stepglass = 0;
  Checkcap = 0;
  Stepclosemachine = 0;
  Stepopenmachine = 0;
  Countcup = 0;
  Step2 = 0;
  Step3 = 0;
  Step4 = 0;
  Step5 = 0;
  Serial.println("main loop");
  Serial.println("Tempid = ");
  Serial.println(Tempid);
  Statephoto2capsule = digitalRead(Photo2);
  Statephoto1glass = digitalRead(Photo1);
  messageupdatestatus = String("{") + String('"')+ String("MachineStatus") + String('"') + String(':') +
                          String('"')+ String("READY") + String('"') + String("}");
                 client.publish("OMC/MSG3", messageupdatestatus.c_str());
  delay(3000);
  while (Statephoto2capsule == 0) { //หากเจอแคปซูลค้าง
    delay(1000);
    Statephoto2capsule = digitalRead(Photo2);
    Statephoto1glass = digitalRead(Photo1);
    Serial.print("Remove Capsule");
    if (Statephoto1glass == 0) {
      Stepglass = 1;
    }
  }
  while (Step1 == 0 && OrderId != Tempid) { 
    messageupdatestatus = String("{") + String('"')+ String("MachineStatus") + String('"') + String(':') +
                          String('"')+ String("NOTREADY") + String('"') + String("}");
                 client.publish("OMC/MSG3", messageupdatestatus.c_str());
    delay(3000);
    if (!client.connected()) {
    reconnect();
  }
  client.loop();
    Readstatusmilksyrup();
    Serial.println("Step 1");
    Serial.println("Stepglass = ");
    Serial.println(Stepglass);
    Serial.println("Step4 = ");
    Serial.println(Step4);
    Serial.println("OrderId = ");
    Serial.println(OrderId);
    Serial.println("Checkcap = ");
    Serial.println(Checkcap);
    Serial.println("Step5 = ");
    Serial.println(Step5);
    Serial.println("Statusmilksyrup = ");
    Serial.println(Statusmilksyrup);
    Serial.println("Stepopenmachine = ");
    Serial.println(Stepopenmachine);
    Serial.println("Stepclosemachine = ");
    Serial.println(Stepclosemachine);
    delay(1000);
    client.loop();
    Statephoto1glass = digitalRead(Photo1);
    Statephoto2capsule = digitalRead(Photo2);
    Statelimitopen = digitalRead(Limitopen);
    Statelimitclose = digitalRead(Limitclose);
    
    while(Stepglass == 0){
       Readstatusstock();
       if (!client.connected()) {
    reconnect();
  }
  client.loop();
       Serial.println("wait ready");
       if(Statuscheck == "READY") {
          Glass();
       }
       delay(3000);
       
    }
    while(Step4 == 0 && Stepglass == 1){
    
      Statelimitopen = digitalRead(Limitopen);
      while (Stepopenmachine == 0) {                         //ถ้าไม่เปิด สั่งเปิดฝาเครื่องจนถึงlimitเปิด
        Machineopen();
      }
      Statelimitopen = digitalRead(Limitopen);
      if(Statelimitopen == 1) {
        Serial.println("step 4 mqtt sentorder type");
        delay(500);
        if (!client.connected()) {
          reconnect();
        }
        client.loop();
        delay(500);
        if(Countcup == 0){
          if (!client.connected()) {
          reconnect();
        }
        client.loop();
        delay(1000);
        messagewater = String(OrderId) + String(' ') + 
                  String(Type) + String(' ') +
                  String('0') + String(' ') +
                  String('0') + String(' ') +
                  String('0') + String(' ');
        client.publish("OMC/Typewater", messagewater.c_str());
        delay(500);
        Step4 = 1;
        }else if(Countcup == 1){
          if (!client.connected()) {
          reconnect();
        }
        client.loop();
          delay(1000);
          messagewater = String(0) + String(' ') + 
                  String(Type) + String(' ') +
                  String('0') + String(' ') +
                  String('0') + String(' ') +
                  String('0') + String(' ');
        client.publish("OMC/Typewater", messagewater.c_str());
        delay(500);
        Step4 = 1;
          }
      }
    
    }
    while(Step5 == 0 && Step4 == 1 && Stepglass == 1 && Stepopenmachine == 1){
      Statephoto2capsule = digitalRead(Photo2);
      Serial.println("step5 capsule detec");
      delay(18000);
      if(Statephoto2capsule == 1){
        Successcapsule();          
      }else if (Statephoto2capsule == 0) {                             //เช็คแคปค้างเกิน2วิจบloop  หรือ ทำเป็นinterrupt
        Errorcapsule();
      }
    
   }
    if (Checkcap == 1 && Stepclosemachine == 0 && Step5 == 1) {
      Serial.println("Step Checkcap+closemachine");
      if(Stepclosemachine == 0){
          delay(3000);
          Machineclose();   //ปิดแล้วส่ง mqtt boardให้นมไซรัป
      }  
    }
    

    //check รับ id statusmilkyrup  เขียนรับแล้วรอloopเช็ค
    Serial.println("wait success");
  
   while(Statusmilksyrup == "SUCCESS" && Statuscheck == "READY" && Stepclosemachine == 1) {
        Serial.println("do success stock");
         messagestock = String(CupAmount) + String(' ') + 
                      String(Countcup+1) + String(' ') +
                      String(OrderId) + String(' ') +
                      String(IsStocking) + String(' ');
        client.publish("OMC/Stock", messagestock.c_str());
        //MQTT id+CupAmount+Countcup
             //เพิ่มwhileรอบอร์ด2ตอบกลับ || รอลิฟต์เข้าที่ให้ทำต่อ
             if (CupAmount == (Countcup+1) && Statusmilksyrup == "SUCCESS") {  //ทำเสร็จรอ Board2ทำStockเสร็จและตอบกลับพร้อมให้ทำต่อ จบloop main
                delay(90000);  //90000 ชั้น3  //60000
                while(Clear == 0){
                  Machineclear();
                }
                if (!client.connected()) {
                  reconnect();
                   }
                client.loop();
                Serial.println("DONE ORDER STOCK");
                delay(500);
                messagemachine = String("UNSUCCESS")+ String(' ');     //เพื่อไม่ใช้เข้าloopซ้ำ
                 client.publish("OMC/Machine", messagemachine.c_str());
                delay(500);
                messageserver = String("{") + String('"')+ String("OrderID") + String('"') + String(':') +
                          String(OrderId) + String(',') + String('"')+ String("Status") + String('"') + 
                          String(':') + String('"')+ String("DONE") + String('"') + String("}");
                 client.publish("OMC/MSG3", messageserver.c_str());
                 
                delay(2000);
                if (!client.connected()) {
                  reconnect();
                   }
                client.loop();
                Readstatusmilksyrup();
                Stepglass = 0;
                Checkcap = 0;
                Stepclosemachine = 0;
                Countcup = 0;
                Stepopenmachine = 0;
                Step4 = 0;
                Step5 = 0;
                Tempid = OrderId;
                Step1 = 1;
            } else if (CupAmount != (Countcup+1) && Statusmilksyrup == "SUCCESS") {  //ทำเสร็จรอ Board2ทำStockเสร็จและตอบกลับพร้อมให้ทำต่อ จบloop รอง
              delay(20000);
              Serial.println("NEXT ORDER STOCK");
                 while (Step3 == 0) {
                     Serial.println("Work Stock");
                     if (!client.connected()) {
    reconnect();
  }
  client.loop();
                    delay(500);
                     messageserver = String("{") + String('"')+ String("OrderId") + String('"') + String(':') +
                               String('"')+ String(OrderId) + String('"') + String(',') + String('"')+ String("Status") + String('"') + 
                               String(':') + String('"')+ String("OnProcess") + String('"') + String("}");
                    client.publish("OMC/MSG3", messageserver.c_str());
                           Countcup++;
                           Stepglass = 0;
                           Checkcap = 0;
                           delay(500);
                           messagemachine = String("UNSUCCESS")+ String(' ');
                 client.publish("OMC/Machine", messagemachine.c_str());
                 delay(2000);
                 if (!client.connected()) {
                  reconnect();
                   }
                client.loop();
                 Readstatusmilksyrup();
                 Serial.println(Statusmilksyrup);
                           Stepopenmachine = 0;
                           Stepclosemachine = 0;
                           Step3 = 1;
                           Step4 = 0;
                           Step5 = 0;
               }
      
              Step3 = 0;
         }
  }
  }
}
