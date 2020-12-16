/*
  LED

  This example creates a BLE peripheral with service that contains a
  characteristic to control an LED.

  The circuit:
  - Arduino MKR WiFi 1010, Arduino Uno WiFi Rev2 board, Arduino Nano 33 IoT,
    Arduino Nano 33 BLE, or Arduino Nano 33 BLE Sense board.

  You can use a generic BLE central app, like LightBlue (iOS and Android) or
  nRF Connect (Android), to interact with the services and characteristics
  created in this sketch.

  This example code is in the public domain.
*/

#include <ArduinoBLE.h>

unsigned long count = 0;

union IMUData {
  struct {
    unsigned long x, y, z, pos;
  };
  byte messageData[16];
};

IMUData data;

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // BLE LED Service

// BLE LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);
BLEUnsignedLongCharacteristic currTime("19B10001-E8F2-537E-4F6C-D104768A1215", BLERead | BLEWrite);

BLECharacteristic packedCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1216", BLERead | BLEWrite | BLENotify, sizeof(IMUData));

BLEDescriptor switchDescriptor("A123", "led_status");
BLEDescriptor timeDescriptor("A456", "elapsed_ticks");

const int ledPin = LED_BUILTIN; // pin to use for the LED

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // set LED pin to output mode
  pinMode(ledPin, OUTPUT);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");

    while (1);
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("LED Arduino");
  BLE.setAdvertisedService(ledService);

  switchCharacteristic.addDescriptor(switchDescriptor);
  currTime.addDescriptor(timeDescriptor);

  // add the characteristic to the service
  ledService.addCharacteristic(switchCharacteristic);
  ledService.addCharacteristic(currTime);
  ledService.addCharacteristic(packedCharacteristic);

  // add service
  BLE.addService(ledService);

  // set the initial value for the characeristic:
  switchCharacteristic.writeValue(0);
  currTime.writeValue(count);

  String address = BLE.address();

  Serial.print("Local address is: ");
  Serial.println(address);

  // start advertising
  BLE.advertise();

  Serial.println("BLE LED Peripheral");
  data.x = millis();
  data.y = 1;
  data.z = 2;
  data.pos = 7;
}

void loop() {
  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());

    // while the central is still connected to peripheral:
    while (central.connected()) {
      data.x = millis();
      data.y++;
      data.z++;
      data.pos++;
//      for (int i = 0; i < 16; i++) {
//        Serial.println(data.messageData);
//      }
      packedCharacteristic.writeValue(data.messageData, 16);
      currTime.writeValue(count);
      // if the remote device wrote to the characteristic,
      // use the value to control the LED:
      if (switchCharacteristic.written()) {
        if (switchCharacteristic.value()) {   // any value other than 0
          Serial.println("LED on");
          digitalWrite(ledPin, HIGH);         // will turn the LED on
        } else {                              // a 0 value
          Serial.println(F("LED off"));
          digitalWrite(ledPin, LOW);          // will turn the LED off
        }
        count++;
      }
    }

    // when the central disconnects, print it out:
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
}