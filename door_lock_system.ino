#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp8266.h>
#define BLYNK_TEMPLATE_ID "TMPLRja1ACw4"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SPI.h>
#include <WiFiClient.h>
#include <MFRC522.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#define RST_PIN         D3         // Configurable, see typical pin layout above
#define SS_PIN          D4         // Configurable, see typical pin layout above
#define BUZZER D8 
#define SERVO D10
#define LED_R D9
#define LED_G D0
#define BLYNK_AUTH_TOKEN "_1STTWJUCtdFvsr76dtSzioqTSh0GW5E" //Enter your blynk auth token
LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
Servo sg90;
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "ZTE_2.4G_xqiREP";//Enter your WIFI name
char pass[] = "E6YDAka5";//Enter your WIFI password
String tagUID = "F3 72 37 30";
int counter = 3;
int invalidRetries = 0;
int rfidMode = 0; // SET initial rfid mode to true (IF TRUE THAT MEANS RFID MODE IS DISABLED)
int doorOverride = 0;
BLYNK_WRITE(V0) {
  rfidMode = param.asInt();
  sg90.write(0);
}
BLYNK_WRITE(V1) {
  int value = param.asInt();
  if(value==1){
    openDoorSystem();
    rfidMode=1;
  }else{
    closeDoorSystem();
    rfidMode=0;
  }
}
void setup() {
  
	Serial.begin(115200);		// Initialize serial communications with the PC
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.begin(ssid,pass);
  while(WiFi.status()!=WL_CONNECTED){
    delay(500);
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
  lcd.begin();
  lcd.backlight();
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  noTone(BUZZER);
  sg90.attach(SERVO);
  sg90.write(0);
	SPI.begin();			
	mfrc522.PCD_Init();		
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
	lcd.clear();
}

void loop() {
    Blynk.run();
    WiFiClient client;
    HTTPClient http;
   lcd.setCursor(0, 0);
    lcd.print("   Door Lock");
    lcd.setCursor(0, 1);
    lcd.print(" Scan Your Tag ");

	if ( ! mfrc522.PICC_IsNewCardPresent()) {
		return;
	}

	// Select one of the cards
	if ( ! mfrc522.PICC_ReadCardSerial()) {
		return;
	}
  if(rfidMode){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  Owner has");
    lcd.setCursor(0, 1);
    lcd.print("denied access");
    digitalWrite(LED_R, HIGH);
    tone(BUZZER, 300);
    
    delay(2000);
    digitalWrite(LED_R, LOW);
    noTone(BUZZER);
    lcd.clear();
  }else {
     
    String tag = "";
    for (byte j = 0; j < mfrc522.uid.size; j++)
    {
      tag.concat(String(mfrc522.uid.uidByte[j] < 0x10 ? " 0" : " "));
      tag.concat(String(mfrc522.uid.uidByte[j], HEX));
    }
    tag.toUpperCase();

    //Checking the card
    if (tag.substring(1) == tagUID)
    {
      // If UID of tag is matched.
      invalidRetries=0;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("  Access Granted");
      digitalWrite(LED_G, HIGH);
      lcd.setCursor(0, 1);
      lcd.print("Unlocking Door");
      sg90.write(160);
      tone(BUZZER, 400);
      delay(2000);
       digitalWrite(LED_G, LOW);
      noTone(BUZZER);
      sg90.write(0);
      lcd.clear();
    }

    else
    {
      // If UID of tag is not matched.
      counter--;
      invalidRetries++;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("  Access denied");
      lcd.setCursor(0, 1);
      lcd.print(String(counter) + " attempt/s left");
      digitalWrite(LED_R, HIGH);
      tone(BUZZER, 300);
      delay(2000);
      digitalWrite(LED_R, LOW);
      noTone(BUZZER);
      lcd.clear();
      Serial.print(counter);
      if(counter==0){
        counter =3;
        
      }
      if(invalidRetries==3){
        http.begin(client,"http://maker.ifttt.com/trigger/entry/json/with/key/mcdd7ioVhfAmj0_Hp6fQSQ04Y9HXAfyiuqitVysx0Xw");
        http.GET();
        http.end();
        invalidRetries=0;
      }
   }
  }
	
   
}
void openDoorSystem(){
   
   sg90.write(160);
   digitalWrite(LED_G, HIGH);
   delay(2000);
    
    
}
void closeDoorSystem(){
   sg90.write(0);
   digitalWrite(LED_G, LOW);
   delay(2000);
}
