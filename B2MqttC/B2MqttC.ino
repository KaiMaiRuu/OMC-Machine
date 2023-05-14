#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "LUCA IoT 2.4GHz";
const char* password = "luca5555";
const char* mqtt_server = "broker.mqttdashboard.com";

int Motor3_R = 4; //ลิฟต์          D2     จ่ายบวกไปลิฟต์ขึ้น
int Motor3_L = 14;//ลิฟต์          D5  
int Motor4_R = 12;//ดันแก้ว        D6     จ่ายบวกไปถอย
int Motor4_L = 15;//ดันแก้ว        D8
int Photo_H = 13;//เช็คลิฟต์ขึ้นสุด    D7 
int Photo_L = 0;//เช็คดันแก้วพร้อม   D3
int Photo_AB = 10;//เก็บแก้วชั้น3    S3
int Photo_B = 9;//เก็บแก้วชั้น2      S2
int Photo_A = 2;//เก็บแก้วชั้น1      D4

int Photo_Hstate = 0;
int Step0_elevator;
int Step0_1_elevetor;
int Step0_2_elevetor;

int Photo_Lstate = 0;
int Step0_pushglass;
int Step0_1_pushglass;

int Photo_Astate = 0;
int Photo_Bstate = 0;
int Photo_ABstate = 0;

int Step1;
int CupAmount = 0;
int Countcup = 0;
int OrderId = 0;
int Tempid = 0;
int Stock = 2;


int Status21 = 0;
int Status22 = 0;
int Status11 = 0;

String strs[5];
int StringCount = 0;
String msgr = "0 0 ";

String messagemachine;
String messageupdatestatus;
int Statusstock;

WiFiClient espClient;
PubSubClient client(espClient);


void setup_wifi() {

  delay(100);
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
  msgr = state;

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
      client.subscribe("OMC/Stock");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void Setelevator1(){      //สั่งdelayแล้วหยุดลิฟต์
    Photo_Hstate = digitalRead(Photo_H);
    if(Photo_Hstate == 0){    //เจอdelayแล้วหยุด
       delay(5100);
       analogWrite(Motor3_R, 0);
       analogWrite(Motor3_L, 0);
       delay(2000);
       Serial.println("Done Step0_1_elevetor");
       Step0_1_elevetor = 1;
       Step0_2_elevetor = 1;
       Step0_elevator = 1;
      }
}

void Setelevator2(){    //สั่งหยุดและให้ลิฟต์ขึ้น
   Photo_Hstate = digitalRead(Photo_H);
   if(Photo_Hstate == 1){         //ไม่เจอแล้วหยุด
      analogWrite(Motor3_R, 0);
      analogWrite(Motor3_L, 0);
      delay(1000);
      analogWrite(Motor3_R, 150);  //ลิฟต์ขึ้น
      analogWrite(Motor3_L, 0);
      delay(5100); 
      analogWrite(Motor3_R, 0);
      analogWrite(Motor3_L, 0);
      delay(2000);
      Serial.println("Done Step0_2_elevetor");
      Step0_2_elevetor = 1;
      Step0_1_elevetor = 1;
      Step0_elevator = 1;
     }
}

void Setpushglass(){ //สั่งหยุด
   while(Step0_pushglass == 0){
      analogWrite(Motor4_R, 100);  //ถอยพาร์ทดันแก้ว                                   //   Setเริ่มต้นให้พาร์ทดันแก้วพร้อม
      analogWrite(Motor4_L, 0);
      delay(4100);
      analogWrite(Motor4_R, 0);
      analogWrite(Motor4_L, 0);
      Serial.println("Done Step0_1_pushglass");
      Step0_1_pushglass = 1;
      Step0_pushglass = 1;
    }
}

void Box1(){
 
  while(CupAmount != 0){
   int Statebox1 = 0;
 while(Statebox1 == 0){
    if (!client.connected()) {
    reconnect();
    }
   client.loop();
   Serial.println("box1 loop");
   Serial.println(CupAmount);
   Serial.println(Countcup);
     
     Readvalue();
     delay(1000);
     //callback mqtt วนรับค่าใหม่ หรือ เทียบค่าเก่าไม่เท่าให้ทำ
     if(Countcup == 1 && CupAmount == 2 && Status21 == 0){
        Serial.println("do 2 1");
        Sendmessagemachine("NOTREADY");
        Stepbox1_1();      
        Statebox1 = 1;
        Status21 = 1;

     }else if(Countcup == 2 && CupAmount == 2 && Status22 == 0){    
        Serial.println("do 2 2");
        Sendmessagemachine("NOTREADY");
        Stepbox1_2();
        Statebox1 = 1;
        Status22 = 1;
        CupAmount = 0;
     }else if(Countcup == 1 && CupAmount == 1 && Status21 == 0){
        Serial.println("do 1 1");
        Sendmessagemachine("NOTREADY");
        Stepbox1_1();
        Statebox1 = 1;
        Status11 = 1;
        CupAmount = 0;
     }
   }
     
  }
  
  Status21 = 0;
  Status22 = 0;
  Status11 = 0;
  Tempid = OrderId;
  Step1 = 1;
}

void Box2(){
  
   while(CupAmount != 0){
   int Statebox2 = 0;
 while(Statebox2 == 0){
    if (!client.connected()) {
    reconnect();
    }
   client.loop();
   Serial.println("box2 loop");
   Serial.println(CupAmount);
   Serial.println(Countcup);
     
     Readvalue();
     delay(1000);
     //callback mqtt วนรับค่าใหม่ หรือ เทียบค่าเก่าไม่เท่าให้ทำ

     if(Countcup == 1 && CupAmount == 2 && Status21 == 0){
        Serial.println("do 2 1");
        Sendmessagemachine("NOTREADY");
        delay(2000);
    analogWrite(Motor3_R, 0);    //ลง
    analogWrite(Motor3_L, 200);
    delay(11250); //ชั้น 1 ไป 2 pwm200 ใช้11200  1 ไป 3 pwm200 ใช้ 22500
    analogWrite(Motor3_R, 0);  
    analogWrite(Motor3_L, 0);
    delay(2000);
        Stepbox2_1();      
        Statebox2 = 1;
        Status21 = 1;

     }else if(Countcup == 2 && CupAmount == 2 && Status22 == 0){    
        Serial.println("do 2 2");
        Sendmessagemachine("NOTREADY");
        delay(2000);
    analogWrite(Motor3_R, 0);    //ลง
    analogWrite(Motor3_L, 200);
    delay(11250); //ชั้น 1 ไป 2 pwm200 ใช้11200  1 ไป 3 pwm200 ใช้ 22500
    analogWrite(Motor3_R, 0);  
    analogWrite(Motor3_L, 0);
    delay(2000);
        Stepbox2_2();
        Statebox2 = 1;
        Status22 = 1;
        CupAmount = 0;
     }else if(Countcup == 1 && CupAmount == 1 && Status21 == 0){
        Serial.println("do 1 1");
        Sendmessagemachine("NOTREADY");
        delay(2000);
    analogWrite(Motor3_R, 0);    //ลง
    analogWrite(Motor3_L, 200);
    delay(11250); //ชั้น 1 ไป 2 pwm200 ใช้11200  1 ไป 3 pwm200 ใช้ 22500
    analogWrite(Motor3_R, 0);  
    analogWrite(Motor3_L, 0);
    delay(2000);
        Stepbox2_1();      
        Statebox2 = 1;
        Status11 = 1;
        CupAmount = 0;
     }
   }
  
  }
  
  Status21 = 0;
  Status22 = 0;
  Status11 = 0;
  Tempid = OrderId;
  Step1 = 1;
}

void Box3(){
   while(CupAmount != 0){
   int Statebox3 = 0;
 while(Statebox3 == 0){
    if (!client.connected()) {
    reconnect();
    }
   client.loop();
   Serial.println("box3 loop");
   Serial.println(CupAmount);
   Serial.println(Countcup);
     
     Readvalue();
     delay(1000);
     //callback mqtt วนรับค่าใหม่ หรือ เทียบค่าเก่าไม่เท่าให้ทำ
     
     if(Countcup == 1 && CupAmount == 2 && Status21 == 0){
        Serial.println("do 2 1");
        Sendmessagemachine("NOTREADY");
        delay(2000);
    analogWrite(Motor3_R, 0);    //ลง
    analogWrite(Motor3_L, 200);
    delay(22500); //ชั้น 1 ไป 2 pwm200 ใช้11200  1 ไป 3 pwm200 ใช้ 22500
    analogWrite(Motor3_R, 0);  
    analogWrite(Motor3_L, 0);
    delay(2000);
        Stepbox2_1();      
        Statebox3 = 1;
        Status21 = 1;

     }else if(Countcup == 2 && CupAmount == 2 && Status22 == 0){    
        Serial.println("do 2 2");
        Sendmessagemachine("NOTREADY");
        delay(2000);
    analogWrite(Motor3_R, 0);    //ลง
    analogWrite(Motor3_L, 200);
    delay(22500); //ชั้น 1 ไป 2 pwm200 ใช้11200  1 ไป 3 pwm200 ใช้ 22500
    analogWrite(Motor3_R, 0);  
    analogWrite(Motor3_L, 0);
    delay(2000);
        Stepbox2_2();
        Statebox3 = 1;
        Status22 = 1;
        CupAmount = 0;
     }else if(Countcup == 1 && CupAmount == 1 && Status21 == 0){
        Serial.println("do 1 1");
        Sendmessagemachine("NOTREADY");
        delay(2000);
    analogWrite(Motor3_R, 0);    //ลง
    analogWrite(Motor3_L, 200);
    delay(22500); //ชั้น 1 ไป 2 pwm200 ใช้11200  1 ไป 3 pwm200 ใช้ 22500
    analogWrite(Motor3_R, 0);  
    analogWrite(Motor3_L, 0);
    delay(2000);
        Stepbox2_1();      
        Statebox3 = 1;
        Status11 = 1;
        CupAmount = 0;
     }
   }
   
  }
  
  Status21 = 0;
  Status22 = 0;
  Status11 = 0;
  Tempid = OrderId;
  Step1 = 1;
}

void Stepbox1_2(){   //แก้วตำแหน่ง2
        analogWrite(Motor4_R, 0);           
        analogWrite(Motor4_L, 0);
        delay(2000);
        analogWrite(Motor4_R, 0);  //ดันพาร์ทดันแก้ว          
        analogWrite(Motor4_L, 100);
        delay(2500);  //ตำแหน่งแก้ว1 3500 แก้ว2 2000 
        analogWrite(Motor4_R, 0);           
          analogWrite(Motor4_L, 0);
          delay(1000);
        analogWrite(Motor3_R, 0);    //ลง
        analogWrite(Motor3_L, 200);
        delay(2000); //ชั้น 1 ไป 2 pwm200 ใช้11200  1 ไป 3 pwm200 ใช้ 22500
        analogWrite(Motor3_R, 0);  
        analogWrite(Motor3_L, 0);
        delay(2000);
        analogWrite(Motor4_R, 100);  //ถอยพาร์ทดันแก้ว                                   
        analogWrite(Motor4_L, 0);
        delay(2500); 
        analogWrite(Motor4_R, 0);  
        analogWrite(Motor4_L, 0);
        delay(1000);
      int yy12 =0;
      while(yy12 == 0){
                   Photo_Hstate = digitalRead(Photo_H);
                   if(Photo_Hstate == 0){  //ถ้าเจอลิฟต์
                      Serial.println("Detec down elevator");
                      analogWrite(Motor3_R, 0);
                      analogWrite(Motor3_L, 150);
                   }else if(Photo_Hstate == 1){         //ไม่เจอแล้วหยุด
                      analogWrite(Motor3_R, 0);
                      analogWrite(Motor3_L, 0);
                      delay(1000);
                      analogWrite(Motor3_R, 150);  //ลิฟต์ขึ้น
                      analogWrite(Motor3_L, 0);
                      delay(4800); 
                      analogWrite(Motor3_R, 0);
                      analogWrite(Motor3_L, 0);
                      delay(2000);
                      Serial.println("Done elevetor");
                       Sendmessagemachine("READY");
                      break;
                    }
      }
  }

void Stepbox1_1(){  //แก้วตำแหน่ง1
        analogWrite(Motor4_R, 0);           
        analogWrite(Motor4_L, 0);
        delay(2000);
        analogWrite(Motor4_R, 0);  //ดันพาร์ทดันแก้ว          
        analogWrite(Motor4_L, 100);
        delay(4000);  //ตำแหน่งแก้ว1 3500 แก้ว2 2000 
        analogWrite(Motor4_R, 0);           
          analogWrite(Motor4_L, 0);
          delay(1000);
        analogWrite(Motor3_R, 0);    //ลง
        analogWrite(Motor3_L, 200);
        delay(2000); //ชั้น 1 ไป 2 pwm200 ใช้11200  1 ไป 3 pwm200 ใช้ 22500
        analogWrite(Motor3_R, 0);  
        analogWrite(Motor3_L, 0);
        delay(2000);
        int xx11 =0;  
        while(xx11 == 0){
        analogWrite(Motor4_R, 100);  //ถอยพาร์ทดันแก้ว                                   
        analogWrite(Motor4_L, 0);
        Photo_Lstate = digitalRead(Photo_L);
        if(Photo_Lstate == 0){
          analogWrite(Motor4_R, 0);           
          analogWrite(Motor4_L, 0);
          delay(1000);
          analogWrite(Motor4_R, 100);  //ถอยพาร์ทดันแก้ว                                   
          analogWrite(Motor4_L, 0);
          delay(4100);
          analogWrite(Motor4_R, 0);
          analogWrite(Motor4_L, 0);
          break;
          }
      }
      int yy11 =0;
      while(yy11 == 0){
                   Photo_Hstate = digitalRead(Photo_H);
                   if(Photo_Hstate == 0){  //ถ้าเจอลิฟต์
                      Serial.println("Detec down elevator");
                      analogWrite(Motor3_R, 0);
                      analogWrite(Motor3_L, 150);
                   }else if(Photo_Hstate == 1){         //ไม่เจอแล้วหยุด
                      analogWrite(Motor3_R, 0);
                      analogWrite(Motor3_L, 0);
                      delay(1000);
                      analogWrite(Motor3_R, 150);  //ลิฟต์ขึ้น
                      analogWrite(Motor3_L, 0);
                      delay(4800); 
                      analogWrite(Motor3_R, 0);
                      analogWrite(Motor3_L, 0);
                      delay(2000);
                      Serial.println("Done elevetor");
                       Sendmessagemachine("READY");
                      break;
                    }
      }
  }

void Stepbox2_2(){   //แก้วตำแหน่ง2
        analogWrite(Motor4_R, 0);           
        analogWrite(Motor4_L, 0);
        delay(2000);
        analogWrite(Motor4_R, 0);  //ดันพาร์ทดันแก้ว          
        analogWrite(Motor4_L, 100);
        delay(2500);  //ตำแหน่งแก้ว1 3500 แก้ว2 2000 
        analogWrite(Motor4_R, 0);           
          analogWrite(Motor4_L, 0);
          delay(1000);
        analogWrite(Motor3_R, 0);    //ลง
        analogWrite(Motor3_L, 200);
        delay(2000); //ชั้น 1 ไป 2 pwm200 ใช้11200  1 ไป 3 pwm200 ใช้ 22500
        analogWrite(Motor3_R, 0);  
        analogWrite(Motor3_L, 0);
        delay(2000);
        analogWrite(Motor4_R, 100);  //ถอยพาร์ทดันแก้ว                                   
        analogWrite(Motor4_L, 0);
        delay(2500); 
        analogWrite(Motor4_R, 0);  
        analogWrite(Motor4_L, 0);
        delay(1000);
      int yy12 =0;
      while(yy12 == 0){
                   Photo_Hstate = digitalRead(Photo_H);
                   if(Photo_Hstate == 1){  //ถ้าไม่เจอลิฟต์
                      Serial.println("NoDetec up elevator");
                      analogWrite(Motor3_R, 150);
                      analogWrite(Motor3_L, 0);
                   }else if(Photo_Hstate == 0){         //เจอแล้วหยุด
                      analogWrite(Motor3_R, 0);
                      analogWrite(Motor3_L, 0);
                      delay(1000);
                      analogWrite(Motor3_R, 150);  //ลิฟต์ขึ้น
                      analogWrite(Motor3_L, 0);
                      delay(4800); 
                      analogWrite(Motor3_R, 0);
                      analogWrite(Motor3_L, 0);
                      delay(2000);
                      Serial.println("Done elevetor");
                       Sendmessagemachine("READY");
                      break;
                    }
      }
  }

void Stepbox2_1(){  //แก้วตำแหน่ง1
        analogWrite(Motor4_R, 0);           
        analogWrite(Motor4_L, 0);
        delay(2000);
        analogWrite(Motor4_R, 0);  //ดันพาร์ทดันแก้ว          
        analogWrite(Motor4_L, 100);
        delay(4000);  //ตำแหน่งแก้ว1 3500 แก้ว2 2000 
        analogWrite(Motor4_R, 0);           
          analogWrite(Motor4_L, 0);
          delay(1000);
        analogWrite(Motor3_R, 0);    //ลง
        analogWrite(Motor3_L, 200);
        delay(2000); //ชั้น 1 ไป 2 pwm200 ใช้11200  1 ไป 3 pwm200 ใช้ 22500
        analogWrite(Motor3_R, 0);  
        analogWrite(Motor3_L, 0);
        delay(2000);
        int xx11 =0;  
        while(xx11 == 0){
        analogWrite(Motor4_R, 100);  //ถอยพาร์ทดันแก้ว                                   
        analogWrite(Motor4_L, 0);
        Photo_Lstate = digitalRead(Photo_L);
        if(Photo_Lstate == 0){
          analogWrite(Motor4_R, 0);           
          analogWrite(Motor4_L, 0);
          delay(1000);
          analogWrite(Motor4_R, 100);  //ถอยพาร์ทดันแก้ว                                   
          analogWrite(Motor4_L, 0);
          delay(4100);
          analogWrite(Motor4_R, 0);
          analogWrite(Motor4_L, 0);
          break;
          }
      }
      int yy11 =0;
      while(yy11 == 0){
                   Photo_Hstate = digitalRead(Photo_H);
                   if(Photo_Hstate == 1){  //ถ้าไม่เจอลิฟต์
                      Serial.println("NoDetec up elevator");
                      analogWrite(Motor3_R, 150);
                      analogWrite(Motor3_L, 0);
                   }else if(Photo_Hstate == 0){         //เจอแล้วหยุด
                      analogWrite(Motor3_R, 0);
                      analogWrite(Motor3_L, 0);
                      delay(1000);
                      analogWrite(Motor3_R, 150);  //ลิฟต์ขึ้น
                      analogWrite(Motor3_L, 0);
                      delay(4800); 
                      analogWrite(Motor3_R, 0);
                      analogWrite(Motor3_L, 0);
                      delay(2000);
                      Serial.println("Done elevetor");
                       Sendmessagemachine("READY");
                      break;
                    }
      }
  }

void Readvalue(){
         while (msgr.length() > 0){
            int index = msgr.indexOf(' ');
            if(index == -1){     // No space found
               strs[StringCount++] = msgr;
               break;
            }else{
               strs[StringCount++] = msgr.substring(0, index);
               msgr = msgr.substring(index+1);
            }
            Serial.println("Struc readstring");
         }
        StringCount = 0;
  /*    for (int i = 0; i < StringCount; i++)
          {
            Serial.print(i);
            Serial.print(": \"");
            Serial.print(strs[i]);                          //แสดงค่ารับ
            Serial.println("\"");
        } 
        */
        CupAmount = strs[0].toInt();
        Countcup = strs[1].toInt();
        OrderId = strs[2].toInt();
        Stock = strs[3].toInt();
      
}

void Sendmessagemachine(String Message){
    Photo_Astate = digitalRead(Photo_A);
    Photo_Bstate = digitalRead(Photo_B);
    Photo_ABstate = digitalRead(Photo_AB);
     if(Photo_Bstate == 1 || Photo_ABstate == 1){
         Statusstock = 1;
     }else{
        Statusstock = 0;
     }   
     delay(500);
     if (!client.connected()) {
    reconnect();
    }
   client.loop();
   delay(500);
     messagemachine = String(Message)+ String(' ') + String(Statusstock) + String(' ') ;
                 client.publish("OMC/Machine", messagemachine.c_str());
   delay(500);
     messageupdatestatus = String("{") + String('"')+ String("MachineStockStatus") + String('"') + String(':') + 
                         String('"') + String(Statusstock) + String('"') + String("}");
                 client.publish("OMC/MSG3", messageupdatestatus.c_str());
  }

void setup() {
  pinMode(Motor3_R, OUTPUT);//D2
  pinMode(Motor3_L, OUTPUT);//D5
  pinMode(Motor4_R, OUTPUT);//D6
  pinMode(Motor4_L, OUTPUT);//D8
  pinMode(Photo_H, INPUT); //D7
  pinMode(Photo_L, INPUT); //D3
  pinMode(Photo_AB, INPUT); //S3
  pinMode(Photo_B, INPUT); //S2
  pinMode(Photo_A, INPUT); //D4
 
  
 
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  Step0_pushglass = 0;                                                         //--------------------------------------------
   Step0_1_pushglass = 0;

   while(Step0_1_pushglass == 0){  //วนจนStep0_1_pushglass = 1
                Serial.println("Do loop Step0_1_pushglass");
                Photo_Lstate = digitalRead(Photo_L);
                   if(Photo_Lstate == 0){    //ถ้าเจอทำ
                      Serial.println("detec pushglass");
                      analogWrite(Motor4_R, 0);  //ดันพาร์ทดันแก้ว                                   //   Setเริ่มต้นให้พาร์ทดันแก้วพร้อม
                      analogWrite(Motor4_L, 100);
                   }else if(Photo_Lstate == 1){    //ไม่เจอแล้วหยุด
                      analogWrite(Motor4_R, 0);
                      analogWrite(Motor4_L, 0);
                      delay(1000);
                      Setpushglass();
                      break;
                      }
                   
             }                                                //-------------------------------------------------  

   
  Step0_elevator = 0;                                                           //-------------------------------------------------
  Step0_1_elevetor = 0;                                                                      //
  Step0_2_elevetor = 0;                                                                      //
  while(Step0_elevator == 0){   
    delay(1000);                                                                
      Serial.println("Startcheck elevatoe");
      Photo_Hstate = digitalRead(Photo_H);
         if(Photo_Hstate == 1){     //photoไม่เจอลิฟต์
            Serial.println("Notdetec elevator");
            while(Step0_1_elevetor == 0){     //วนจนStep0_1_elevetor = 1    
               Serial.println("Do loop Step0_1_elevetor");
               Photo_Hstate = digitalRead(Photo_H);
                   if(Photo_Hstate == 1){   //ถ้าไม่เจอทำ
                     Serial.println("Notdetec up elevator");
                     analogWrite(Motor3_R, 150);  //ลิฟต์ขึ้น                                   //   Setเริ่มต้นให้ลิฟต์พร้อม
                     analogWrite(Motor3_L, 0);
                   }else if(Photo_Hstate == 0){
                Setelevator1();
                }
            }
         }else if(Photo_Hstate == 0){    //photoเจอลิฟต์
            Serial.println("Detec elevator");
            while(Step0_2_elevetor == 0){
              delay(1000);     
               Serial.println("Do loop Step0_2_elevetor");
               Photo_Hstate = digitalRead(Photo_H);
                   if(Photo_Hstate == 0){  //ถ้าเจอลิฟต์
                      Serial.println("Detec down elevator");
                      analogWrite(Motor3_R, 0);
                      analogWrite(Motor3_L, 150);
                   } 
                Setelevator2();
            }
        }                                                                                   //
                                                                                            //
   }                                                                            //--------------------------------------------
   Photo_Bstate = digitalRead(Photo_B);
   Photo_ABstate = digitalRead(Photo_AB);
   if(Photo_Bstate == 0 && Photo_ABstate == 0 ){
      //MQTT Sent stock NOtready
      messageupdatestatus = String("{") + String('"')+ String("IsStock") + String('"') + String(':') +
                         String("0") + String("}");
                 client.publish("OMC/MSG3", messageupdatestatus.c_str());
      Serial.println("Stock Notready");
   }else if(Photo_Bstate == 1 || Photo_ABstate == 1 ){
      //MQTT Sent stock ready
      messageupdatestatus = String("{") + String('"')+ String("MachineStockStatus") + String('"') + String(':') +
                          String('"') + String("1") + String('"') + String("}");
                 client.publish("OMC/MSG3", messageupdatestatus.c_str());
      Serial.println("Stock Ready");
   }

  
  Status21 = 0;
  Status22 = 0;
  Status11 = 0;
   Sendmessagemachine("READY");
  
}
void loop() {

   if (!client.connected()) {
    reconnect();
    }
   client.loop();
    Photo_Hstate = digitalRead(Photo_H);
    Photo_Lstate = digitalRead(Photo_L);
    Photo_Astate = digitalRead(Photo_A);
    Photo_Bstate = digitalRead(Photo_B);
    Photo_ABstate = digitalRead(Photo_AB);

    //รับ กี่แก้ว แก้วที่
    Step1 = 0;
    Readvalue();
    Serial.println("main loop");
    Serial.println(Photo_Astate);
    Serial.println(Photo_Bstate);
    Serial.println(Photo_ABstate);
    delay(1000);
       while(Step1 == 0 && CupAmount != 0 && OrderId != Tempid){
        Photo_Astate = digitalRead(Photo_A);
        Photo_Bstate = digitalRead(Photo_B);
        Photo_ABstate = digitalRead(Photo_AB); 
          if (!client.connected()) {
              reconnect();
          }
          client.loop();
          delay(1000);
          if(Stock == 0 ){
         
             Box1();
          
          }else if(Photo_Bstate == 1 && Stock == 1){
           
             Box2();
          
          }else if(Photo_Bstate == 0 && Photo_ABstate == 1 && Stock == 1){
        
             Box3();
          
          }
         
       }
       

   
}
