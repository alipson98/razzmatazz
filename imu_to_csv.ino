/**
 * @file mvp.ino
 * @brief get imu data and transmit it over bluetooth
 */

#include <ArduinoBLE.h>

// Basic demo for readings from Adafruit BNO08x
#include <Adafruit_BNO08x.h>

// For SPI mode, we need a CS pin
#define BNO08X_CS 10
#define BNO08X_INT 9
#define BNO08X_RESET 5

Adafruit_BNO08x  bno08x(BNO08X_RESET);
sh2_SensorValue_t sensorValue;

#define DATA_SIZE 16 // change according to IMUData

union IMUData {
  struct {
    unsigned long curr_time;
    float gyro_x, gyro_y, gyro_z;
  };
  byte messageData[DATA_SIZE];
};

IMUData data;

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // BLE LED Service

// BLE LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLECharacteristic packedCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1216", BLERead | BLEWrite | BLENotify, sizeof(IMUData));

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
  BLE.setLocalName("IMU Arduino");
  BLE.setAdvertisedService(ledService);

  // add the characteristic to the service
  ledService.addCharacteristic(packedCharacteristic);

  // add service
  BLE.addService(ledService);

  // set the initial value for the characeristic:


  String address = BLE.address();

  Serial.print("Local address is: ");
  Serial.println(address);

  // start advertising
  BLE.advertise();

  Serial.println("BLE LED Peripheral");
  data.curr_time = millis();
  data.gyro_x = 0;
  data.gyro_y = 0;
  data.gyro_z = 0;


  Serial.println("Adafruit BNO08x test!");

  // Try to initialize!
  if (!bno08x.begin_I2C()) {
  //if (!bno08x.begin_UART(&Serial1)) {  // Requires a device with > 300 byte UART buffer!
  //if (!bno08x.begin_SPI(BNO08X_CS, BNO08X_INT)) {
    Serial.println("Failed to find BNO08x chip");
    while (1) { delay(10); }
  }
  Serial.println("BNO08x Found!");

  for (int n = 0; n < bno08x.prodIds.numEntries; n++) {
    Serial.print("Part ");
    Serial.print(bno08x.prodIds.entry[n].swPartNumber);
    Serial.print(": Version :");
    Serial.print(bno08x.prodIds.entry[n].swVersionMajor);
    Serial.print(".");
    Serial.print(bno08x.prodIds.entry[n].swVersionMinor);
    Serial.print(".");
    Serial.print(bno08x.prodIds.entry[n].swVersionPatch);
    Serial.print(" Build ");
    Serial.println(bno08x.prodIds.entry[n].swBuildNumber);
  }

  setReports();

  Serial.println("Reading events");
  delay(100);
}

// Here is where you define the sensor outputs you want to receive
void setReports(void) {
  Serial.println("Setting desired reports");
  if (! bno08x.enableReport(SH2_GYROSCOPE_CALIBRATED)) {
    Serial.println("Could not enable game vector");
  }
}

void loop() {
  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();


  if (bno08x.wasReset()) {
    Serial.print("sensor was reset ");
    setReports();
  }
  
  if (! bno08x.getSensorEvent(&sensorValue)) {
    return;
  }

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());

    // while the central is still connected to peripheral:
    while (central.connected()) {

        switch (sensorValue.sensorId) {
            case SH2_GYROSCOPE_CALIBRATED: {
            data.curr_time = millis();
            data.gyro_x = sensorValue.un.gyroscope.x;
            data.gyro_y = sensorValue.un.gyroscope.y;
            data.gyro_z = sensorValue.un.gyroscope.z;
            if (data.gyro_x > 1 or data.gyro_x < -1 or data.gyro_y > 1 or data.gyro_y < -1 or data.gyro_z > 1 or data.gyro_z < -1) {
                Serial.print("Gyro - x: ");
                Serial.print(data.gyro_x);
                Serial.print(" y: ");
                Serial.print(data.gyro_y);
                Serial.print(" z: ");
                Serial.println(data.gyro_z);
            }
            break;

            }
        }
      packedCharacteristic.writeValue(data.messageData, DATA_SIZE);
    }

    // when the central disconnects, print it out:
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
}