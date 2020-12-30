//Fingerprint Serial1
#include <Adafruit_Fingerprint.h>
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial1);

int idFinger;
int count;

//Relay
byte kontak = 52;
byte starter = 53;

//GPS Serial2
#include <TinyGPS++.h>
TinyGPSPlus gps;
double latitude, longitude;

//SIM800L
#include <SoftwareSerial.h>
SoftwareSerial sim(12, 13);
String response;
int lastStringLength = response.length(); 
String nohape = "088224132138"; //No HP Penerima

int counttracker;

void setup() {
  Serial.begin(9600);
  //Fingerprint Begin
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("FingerPrint Sensor Ditemukan!");
  }
  else {
    Serial.println("FingerPrint Sensor Tidak Ditemukan! :(");
    while (1) {
      delay(1);
    }
  }

  //Relay Inisialisasi
  pinMode(kontak, OUTPUT);
  pinMode(starter, OUTPUT);
  digitalWrite(kontak, LOW);
  digitalWrite(starter, LOW);

  //GPS Begin
  Serial2.begin(9600);
  Serial.println("GPS Mulai");

  //SIM Begin
  sim.begin(9600);
  sim.println("AT+CMGF=1");
  Serial.println("sim started at 9600");
  delay(1000);
  Serial.println("Setup Complete! sim is Ready!");
  sim.println("AT+CNMI=2,2,0,0,0");
}

void loop() {
  FINGERPRINT();
  if (idFinger >= 0) {
    count++;
    if (count == 1) {
      digitalWrite(kontak, HIGH);
    }
    if (count == 2) {
      digitalWrite(starter, HIGH);
      delay(1500);
      digitalWrite(starter, LOW);
    }
    if (count == 3) {
      digitalWrite(kontak, LOW);
      count = 0;
    }
  }

  if (Serial.available()) {
    sim.write(Serial.read());
  }
  if (sim.available() > 0) {
    response = sim.readStringUntil('\n');
  }
  if (lastStringLength != response.length()) {
    Serial.println(response);
    if (response.indexOf("Track") == 0) {
      counttracker = 1;
    }
    lastStringLength = response.length();
  }

  while (Serial2.available()) {
    gps.encode(Serial2.read());
  }
  if (gps.location.isUpdated()) {
    latitude = gps.location.lat();
    longitude = gps.location.lng();
    String link = "https://www.google.com/maps/place/" + String(latitude, 6) + "," + String(longitude, 6) ;
    Serial.println(link);
    if (counttracker == 1) {
      sim.println("AT+CMGF=1");
      delay(1000);
      sim.print("AT+CMGS=\"");
      sim.print(nohape);
      sim.println("\"\r");
      delay(1000);
      sim.println(link);
      delay(100);
      sim.println((char)26);
      delay(1000);
      counttracker = 0;
    }
  }
}
//----------------------------------------------------FINGERPRINT----------------------------------------------//
void FINGERPRINT() {
  idFinger = getFingerprintIDez();
  delay(50);
}

//----------------------------------------------------PROSES FINGERPRINT--------------------------------------//

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);

  return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}

//--------------------------------END FINGERPRINT----------------------------------------------//
