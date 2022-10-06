#include <dummy.h>
#include <Servo.h>
#include <deprecated.h>
#include <MFRC522.h>
#include <MFRC522Extended.h>
#include <require_cpp11.h>

#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN D2  
#define RST_PIN D1  
#define inputpin D3

MFRC522 mfrc522(SS_PIN, RST_PIN);  

#define GREEN_LED D8  
const char* ssid = "iBall-Baton";
const char* password = "querty123";

ESP8266WebServer server(80);  
Servo servo;

int readsuccess;
String content= "";
byte readcard[4];
char str[32] = "";
String StrUID;
bool ir;
int speakerPin = 16;

void setup() {

  Serial.begin(115200); 
  SPI.begin();      
  mfrc522.PCD_Init(); 

  delay(500);

  WiFi.begin(ssid, password); 
  Serial.println("");
   
  pinMode(GREEN_LED,OUTPUT);
  digitalWrite(GREEN_LED,HIGH); 
//----------------------------------------Wait for connection
  Serial.print("Connecting");

  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Please tag a card or keychain to see the UID !");
  Serial.println("");
  servo.attach(2);
  servo.write(0);
  pinMode(inputpin,INPUT);
  pinMode(speakerPin, OUTPUT);
}

void loop() {
  
  readsuccess = getid();
 
  if(readsuccess) {  
  
    HTTPClient http;    
 
    String UIDresultSend, postData;
    UIDresultSend = StrUID;
   
    
    postData = "UIDresult=" + UIDresultSend;
 
    http.begin("http://192.168.1.206:3307/NodeMCU/getUID.php");  
    http.addHeader("Content-Type", "application/x-www-form-urlencoded"); 
   
    int httpCode = http.POST(postData);   
    String payload = http.getString();    
    Serial.println(UIDresultSend);
    Serial.println(httpCode);   
    Serial.println(payload);    
    
  if(payload.toInt()>10){
    Serial.println("Open the door");
    servo.write (90);
    delay(10000);
     
    ir = digitalRead(inputpin);
      if(ir == 0) {
        Serial.println("car gone,close the door");     
        servo.write (0);            
     }
  }
  else if(payload.toInt() == 0){
     digitalWrite(speakerPin, HIGH);
     Serial.println("Recharge your card"); 
    }
  if(payload=="Invalid user. Register your card."){
    digitalWrite(speakerPin, HIGH);
   delay(2000);
    digitalWrite(speakerPin, LOW);
}
    http.end();  
  }
}

int getid() {  
  if(!mfrc522.PICC_IsNewCardPresent()) {
    return 0;
  }
  if(!mfrc522.PICC_ReadCardSerial()) {
    return 0;
  }
 
 
  Serial.print("THE UID OF THE SCANNED CARD IS : ");
 
  for(int i=0;i<4;i++){
    readcard[i]=mfrc522.uid.uidByte[i]; //storing the UID of the tag in readcard
    array_to_string(readcard, 4, str);
    StrUID = str;
  }
  mfrc522.PICC_HaltA();
  return 1;
}

void array_to_string(byte array[], unsigned int len, char buffer[]) {
    for (unsigned int i = 0; i < len; i++)
    {
        byte nib1 = (array[i] >> 4) & 0x0F;
        byte nib2 = (array[i] >> 0) & 0x0F;
        buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
        buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
    }
    buffer[len*2] = '\0';
}
