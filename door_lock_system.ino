/*
 * --------------------------------------------------------------------------------------------------------------------
 * Example sketch/program showing how to read data from a PICC to serial.
 * --------------------------------------------------------------------------------------------------------------------
 * This is a MFRC522 library example; for further details and other examples see: https://github.com/miguelbalboa/rfid
 * 
 * Example sketch/program showing how to read data from a PICC (that is: a RFID Tag or Card) using a MFRC522 based RFID
 * Reader on the Arduino SPI interface.
 * 
 * When the Arduino and the MFRC522 module are connected (see the pin layout below), load this sketch into Arduino IDE
 * then verify/compile and upload it. To see the output: use Tools, Serial Monitor of the IDE (hit Ctrl+Shft+M). When
 * you present a PICC (that is: a RFID Tag or Card) at reading distance of the MFRC522 Reader/PCD, the serial output
 * will show the ID/UID, type and any data blocks it can read. Note: you may see "Timeout in communication" messages
 * when removing the PICC from reading distance too early.
 * 
 * If your reader supports it, this sketch/program will read all the PICCs presented (that is: multiple tag reading).
 * So if you stack two or more PICCs on top of each other and present them to the reader, it will first output all
 * details of the first and then the next PICC. Note that this may take some time as all data blocks are dumped, so
 * keep the PICCs at reading distance until complete.
 * 
 * @license Released into the public domain.
 * 
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 * More pin layouts for other boards can be found here: https://github.com/miguelbalboa/rfid#pin-layout
 */
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
LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
Servo sg90;
char ssid[] = "ZTE_2.4G_xqiREP";//Enter your WIFI name
char pass[] = "E6YDAka5";//Enter your WIFI password
String tagUID = "F3 72 37 30";
int counter = 0;

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
  pinMode(BUZZER, OUTPUT);
  noTone(BUZZER);
  sg90.attach(SERVO);
  sg90.write(0);
	SPI.begin();			
	mfrc522.PCD_Init();		
	lcd.clear();
}

void loop() {
    WiFiClient client;
    HTTPClient http;
    lcd.setCursor(0, 0);
    lcd.print("   Door Lock");
    lcd.setCursor(0, 1);
    lcd.print(" Scan Your Tag ");
	// Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
	if ( ! mfrc522.PICC_IsNewCardPresent()) {
		return;
	}

	// Select one of the cards
	if ( ! mfrc522.PICC_ReadCardSerial()) {
		return;
	}

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
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Access Granted");
      lcd.setCursor(0, 1);
      lcd.print("Unlocking Door");
      sg90.write(160);
      tone(BUZZER, 100);
      delay(2000);
      noTone(BUZZER);
      sg90.write(0);
      lcd.clear();
    }

    else
    {
      // If UID of tag is not matched.
      counter++;
      lcd.clear();
      lcd.print(counter);
      digitalWrite(LED_R, HIGH);
      tone(BUZZER, 300);
      delay(2000);
      digitalWrite(LED_R, LOW);
      noTone(BUZZER);
      lcd.clear();
      if(counter==3){
         http.begin(client,"https://maker.ifttt.com/trigger/Demo/json/with/key/mcdd7ioVhfAmj0_Hp6fQSQ04Y9HXAfyiuqitVysx0Xw");
         http.GET();
         http.end();
         counter =0;
   
      }
    }
}
