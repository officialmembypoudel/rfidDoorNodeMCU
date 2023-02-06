// Created by Memby, Anil, Subodh, Roshan

#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <EspMQTTClient.h>


#define SS_PIN 15
#define RST_PIN 0
MFRC522 mfrc522(SS_PIN, RST_PIN);

const int speakerPin = 16;
const int doorMotor = 4;
const int pingPin = 2;  //D4 Trigger Pin of Ultrasonic Sensor
const int echoPin = 5;  //D1 Echo Pin of Ultrasonic Sensor
boolean rfidState = false;
boolean doorState = false;

Servo servo;

EspMQTTClient client(
  "Redmi",
  "roshan1105",
  "109.205.181.241",  // MQTT Broker server ip
  // "MQTTUsername",   // Can be omitted if not needed
  // "MQTTPassword",   // Can be omitted if not needed
  "autochalitdoormqtt"  // Client name that uniquely identify your device
);


void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("Approximate your card to the reader...");
  Serial.println();
  digitalWrite(speakerPin, LOW);
  servo.attach(doorMotor);
  servo.write(180);
  delay(500);
}

void onConnectionEstablished() {
  client.subscribe("door", [](const String &payload) {
    Serial.println(payload);
    if (payload == "on") {
      doorState = true;
      servo.write(0);
      delay(100);
    } else if (payload == "off") {
      doorState = false;
      servo.write(180);
      delay(100);
    }
  });

  client.publish("mytopic/test", "This is a message");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    // digitalWrite(wifiConnectionLed, HIGH);
    Serial.print("connected");
  } else {
    // digitalWrite(wifiConnectionLed, LOW);
    Serial.print("Disconnected");
  }
  client.loop();

  long duration, cm;
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(pingPin, LOW);
  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);
  cm = microsecondsToCentimeters(duration);
  // delay(100);

  // Serial.print(cm);
  // Serial.print(" cm");
  // Serial.println();
  // delay(100);


  pinMode(speakerPin, OUTPUT);

  if (doorState == false && rfidState == true) {
    servo.write(0);
    Serial.print("from parent while loop");
    delay(500);
    Serial.print(cm);
    Serial.print(" cm");
    Serial.println();
    if (cm > 20) {
      servo.write(180);
      Serial.print("if cm is greater than 20");
      rfidState = false;
    } else {
      delay(500);
      servo.write(0);
    }
  }

  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print("UID tag :");
  String content = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();
  if (content.substring(1) == "E3 CE 4B 08" || content.substring(1) == "B2 64 3B 1B") {
    digitalWrite(speakerPin, HIGH);
    delay(600);
    digitalWrite(speakerPin, LOW);
    rfidState = true;
    Serial.println("Authorised access");
    Serial.println();
    // delay(3000);
  } else {
    digitalWrite(speakerPin, HIGH);
    delay(200);
    digitalWrite(speakerPin, LOW);
    delay(200);
    digitalWrite(speakerPin, HIGH);
    delay(200);
    digitalWrite(speakerPin, LOW);
    delay(200);
    digitalWrite(speakerPin, HIGH);
    delay(200);
    digitalWrite(speakerPin, LOW);
    Serial.println("Access denied");
    // delay(3000);
  }
}

long microsecondsToCentimeters(long microseconds) {
  return microseconds / 29 / 2;
}