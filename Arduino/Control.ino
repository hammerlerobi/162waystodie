/*
   -----------------------------------------------------------------------------------------
               MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
               Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
   Signal      Pin          Pin           Pin       Pin        Pin              Pin
   -----------------------------------------------------------------------------------------
   RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
   SPI SS      SDA(SS)      10            53        D10        10               10
   SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
   SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
   SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15

   PIN 3 to Darlington for Soleniod
*/

#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9          // Configurable, see typical pin layout above
#define SS_PIN 10         // Configurable, see typical pin layout above

int counter = 0;
int noCardPresentCount  = 15; // numbers of reads no card is present to fire a off event
bool cardIsPreset = false;
byte serialIn;
const int solenoidPin =  2;
unsigned long fireStartMillis = 0;
unsigned long fireDurationMillis = 10;


MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

void setup() {
  Serial.begin(9600);   // Initialize serial communications with the PC
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init(); 
  pinMode(solenoidPin, OUTPUT);// Configure Pin to Solenoid as Output
}

void loop() {
  
 
  if (Serial.available() > 0) {
    serialIn = Serial.read();
  
     //Serial Command from Application Send a String "1"
     // "1" is 49 in ASCII
    if (serialIn == 49) {
      Serial.print("LAUNCHJESUITE");
      Serial.print( '\r' );
      fireStartMillis =  millis();
    }
  }

  //Set the SolenoidPin high for fireDurationMillis
  unsigned long currentMillis = millis();
  if (currentMillis - fireStartMillis <= fireDurationMillis) {
    digitalWrite(solenoidPin, HIGH);
   
    Serial.print("fire");
  } else {
    digitalWrite(solenoidPin, LOW);
  }

  //RFID Stuff, we use the PICC_IsNewCardPresent which for some reason outputs 1 and in the next loop 0 if a card is present and constant 0 if no card is present
  if (mfrc522.PICC_IsNewCardPresent()) {
    if (mfrc522.PICC_ReadCardSerial()) {
      counter = 0;
      if (cardIsPreset == false) {
        Serial.print("ON;");
        cardIsPreset = true;
        dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
        Serial.print( '\r' );
      }
    }
  } else {
    counter++;
    if (counter > noCardPresentCount && cardIsPreset) {
      cardIsPreset = false;
      Serial.print("OFF");
      Serial.print( '\r' );
    }
  }
}

//Helper function to write the UID as HEX
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? "0" : "");
    Serial.print(buffer[i], HEX);
  }
}
