// DEFINES
// Provides debugging information over serial connection if defined
#define DEBUG

// LIBRARIES
// SPI library
#include <SPI.h>
// Download and install from https://github.com/miguelbalboa/rfid
#include <MFRC522.h>
// LiquidCrystal_I2C library for the LCD display
#include <LiquidCrystal_I2C.h>

// CONSTANTS
// The number of RFID readers
const byte numReaders = 4;
// Each reader has a unique Slave Select pin
const byte ssPins[] = {2, 3, 4, 5};
// They'll share the same reset pin
const byte resetPin = 8;
// The sequence of NFC tag IDs required to solve the puzzle
const String correctIDs[] = {"53707434", "53b25934", "322cd91e", "53609634"};
// The custom characters/symbols for LCD
//symbol chcek
const byte Check[8] = {
  0b00000,
  0b00001,
  0b00011,
  0b10110,
  0b11100,
  0b01000,
  0b00000,
  0b00000
};
//symbol cross
const byte Cross[8] = {
  0b00000,
  0b00000,
  0b10001,
  0b01010,
  0b00100,
  0b01010,
  0b10001,
  0b00000
};
//symbol heart
const byte Heart[8] = {
  0b00000,
  0b01010,
  0b11111,
  0b11111,
  0b01110,
  0b00100,
  0b00000,
  0b00000
};
//symbol square
const byte Square[8] = {
  0b00000,
  0b00000,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b00000
};
//symbol triangle
const byte Triangle[8] = {
  0b00000,
  0b00000,
  0b00001,
  0b00011,
  0b00111,
  0b01111,
  0b11111,
  0b00000
};
//symbol star
const byte Star[8] = {
  0b00000,
  0b00100,
  0b10101,
  0b01110,
  0b01010,
  0b10001,
  0b00000,
  0b00000
};
// GLOBALS
// object with 3 parameter(adress, columns,rows) - adress, dimensions of the display
LiquidCrystal_I2C lcd(0x27, 16, 2);
// Initialise an array of MFRC522 instances representing each reader
MFRC522 mfrc522[numReaders];
// The tag IDs currently detected by each reader
String currentIDs[numReaders];
/**
 * Initialisation
 */
void setup() {

  #ifdef DEBUG
  // Initialise serial communications channel with the PC
  Serial.begin(9600);
  Serial.println(F("Serial communication started"));
  #endif
  
  // reader's select pin as HIGH - don't cause interference on the SPI bus
  for (uint8_t i=0; i<numReaders; i++) {
    pinMode(ssPins[i], OUTPUT);
    digitalWrite(ssPins[i], HIGH);
  }
  
  // Initialise the SPI bus
  SPI.begin();

  for (uint8_t i=0; i<numReaders; i++) {
  
    // Initialise the reader
    mfrc522[i].PCD_Init(ssPins[i], resetPin);
    
    // Set the gain to max (if necessary)
    //mfrc522[i].PCD_SetAntennaGain(MFRC522::PCD_RxGain::RxGain_max);
    
  #ifdef DEBUG
    // some information to the serial monitor
    Serial.print(F("Reader #"));
    Serial.print(i);
    Serial.print(F(" initialised on pin "));
    Serial.print(String(ssPins[i]));
    Serial.print(F(". Antenna strength: "));
    Serial.print(mfrc522[i].PCD_GetAntennaGain());
    Serial.print(F(". Version : "));
    mfrc522[i].PCD_DumpVersionToSerial();
  #endif
    
    // Slight delay before activating next reader
    delay(100);
  }
  
  #ifdef DEBUG
  Serial.println(F("--- END SETUP ---"));
  #endif

  // displaying some information on the LCD display
  //initialise the LCD
  lcd.init();
  // backlight on       
  lcd.backlight();
  // create a the characters
  lcd.createChar(0, Heart);
  lcd.createChar(1, Star);
  lcd.createChar(2, Square);
  lcd.createChar(3, Triangle);
  lcd.createChar(4, Check);
  lcd.createChar(5, Cross);
  // Clears the LCD screen
  lcd.clear();
  // Print a message to the lcd.
  lcd.setCursor(2, 0);
  lcd.print("Puzzle Game");
  lcd.setCursor(4, 1);
  lcd.write(0);
  lcd.setCursor(6, 1);
  lcd.write(1);
  lcd.setCursor(8, 1);
  lcd.write(2);
  lcd.setCursor(10, 1);
  lcd.write(3);
}

/**
 * Main loop
 */
void loop() {

  // Assume that the puzzle has been solved
  boolean puzzleSolved = true;

  // Assume that the tags have not changed since last reading
  boolean changedValue = false;

  // Loop through each reader
  for (uint8_t i=0; i<numReaders; i++) {

    // Initialise the sensor
    mfrc522[i].PCD_Init();
    
    // String to hold the ID detected by each sensor
    String readRFID = "";
    
    // If the sensor detects a tag and is able to read it
    if(mfrc522[i].PICC_IsNewCardPresent() && mfrc522[i].PICC_ReadCardSerial()) {
      // Extract the ID from the tag
      readRFID = dump_byte_array(mfrc522[i].uid.uidByte, mfrc522[i].uid.size);
    }
    
    // If the current reading is different from the last known reading
    if(readRFID != currentIDs[i]){
      // Set the flag to show that the puzzle state has changed
      changedValue = true;
      // Update the stored value for this sensor
      currentIDs[i] = readRFID;
    }
    
    // If the reading fails to match the correct ID for this sensor 
    if(currentIDs[i] != correctIDs[i]) {
      // The puzzle has not been solved
      puzzleSolved = false;
    }

    // Halt PICC
    mfrc522[i].PICC_HaltA();
    // Stop encryption on PCD
    mfrc522[i].PCD_StopCrypto1(); 
  }

  #ifdef DEBUG
  // If the changedValue flag has been set, at least one sensor has changed
  if(changedValue){
    // show the current state of all sensors on serial monitor
    for (uint8_t i=0; i<numReaders; i++) {
      Serial.print(F("Reader #"));
      Serial.print(String(i));
      Serial.print(F(" on Pin #"));
      Serial.print(String((ssPins[i])));
      Serial.print(F(" detected tag: "));
      Serial.println(currentIDs[i]);

      //show the current state of all sensors on LCD display
      if(ssPins[i] == 2 && currentIDs[i]== "53707434"){
        lcd.clear();
        lcd.setCursor(1, 1);
        lcd.write(4);
        lcd.setCursor(2, 1);
        lcd.write(0);
      }else if(ssPins[i] == 2 && currentIDs[i] != "53707434"){
        lcd.clear();
        lcd.setCursor(1, 1);
        lcd.write(5);
        lcd.setCursor(2, 1);
        lcd.write(0);
      }
      
      if(ssPins[i] == 3 && currentIDs[i]== "53b25934"){
        lcd.setCursor(10, 1);
        lcd.write(4);
        lcd.setCursor(11, 1);
        lcd.write(2);
      }else if(ssPins[i] == 3 && currentIDs[i] != "53b25934"){
        lcd.setCursor(10, 1);
        lcd.write(5);
        lcd.setCursor(11, 1);
        lcd.write(2);
      }

      if(ssPins[i] == 4 && currentIDs[i]== "322cd91e"){
        lcd.setCursor(10, 0);
        lcd.write(4);
        lcd.setCursor(11, 0);
        lcd.write(3);
      }else if(ssPins[i] == 4 && currentIDs[i] != "322cd91e"){
        lcd.setCursor(10, 0);
        lcd.write(5);
        lcd.setCursor(11, 0);
        lcd.write(3);
      }

      if(ssPins[i] == 5 && currentIDs[i]== "53609634"){
        lcd.setCursor(1, 0);
        lcd.write(4);
        lcd.setCursor(2, 0);
        lcd.write(1);
      }else if(ssPins[i] == 5 && currentIDs[i] != "53609634"){
        lcd.setCursor(1, 0);
        lcd.write(5);
        lcd.setCursor(2, 0);
        lcd.write(1);
      }      
      };
    Serial.println(F("---"));    
  }
  #endif

  // If the puzzleSolved flag is set, all sensors detected the correct ID
  if(puzzleSolved){
    onSolve();
  }
  //after the solution, if one object is removed the game continues
  digitalWrite(resetPin, HIGH);
}
/**
 * Called when correct puzzle solution has been entered
 */
void onSolve(){

  #ifdef DEBUG
  // Print message on serial monitor
  Serial.println(F("Puzzle Solved!"));
  #endif

  // Clears the LCD screen
  lcd.clear();
  // Print a message to the LCD
  lcd.print("Puzzle Solved!");  
}
/**
 * Helper function to return a string ID from byte array
 */
String dump_byte_array(byte *buffer, byte bufferSize) {
  String read_rfid = "";
  for (byte i=0; i<bufferSize; i++) {
    read_rfid = read_rfid + String(buffer[i], HEX);
  }
  return read_rfid;
}
