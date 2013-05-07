/*
* Socket App
 *
 * A simple socket application example using the WiShield 1.0
 */

#include <WiShield.h>
#include "Adafruit_BMP085.h"
#include "Wire.h"
#include <Servo.h> 
#include "MPU6050_6Axis_MotionApps20.h"
#include "I2Cdev.h"


//Wireless configuration defines ----------------------------------------
#define WIRELESS_MODE_INFRA   1
#define WIRELESS_MODE_ADHOC   2

//Wireless configuration parameters ----------------------------------------
unsigned char local_ip[]       = {
  192,168,1,136};  // IP address of WiShield
unsigned char gateway_ip[]     = {
  192,168,1,1};     // router or gateway IP address
unsigned char subnet_mask[]    = {
  255,255,255,0}; // subnet mask for the local network
const prog_char ssid[] PROGMEM = {
  "UUV"};    // max 32 bytes
unsigned char security_type    = 0;               // 0 - open; 1 - WEP; 2 - WPA; 3 - WPA2
// WPA/WPA2 passphrase
const prog_char security_passphrase[] PROGMEM = {
  "like40ninjas"};   // max 64 characters
// WEP 128-bit keys
prog_uchar wep_keys[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Key 0
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Key 1
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Key 2
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Key 3
// setup the wireless mode
// infrastructure - connect to AP
// adhoc - connect to another WiFi device
unsigned char wireless_mode = WIRELESS_MODE_INFRA;
unsigned char ssid_len;
unsigned char security_passphrase_len;
// End of wireless configuration parameters ----------------------------------------

char recvChar;
char sms[8];
char outBuffer[40];
char charBuf[30];
Adafruit_BMP085 bmp;
MPU6050 mpu;

// Time interval to send data to Android (milliseconds)
long interval = 1000;
long prevTime;

//On/Off
int ledPin = 13;

// Accelerometer
int xPin = 13;
int yPin = 13;
int zPin = 13;

// Gyroscope
int xGyPin = 13;
int yGyPin = 13;
int zGyPin = 13;

// Movement
int risePin = 13;
int fallPin = 13;
int forwardPin = 13;
int backwardPin = 13;

// Actuators
int frontActuator = 30;
int frontActuatorSwitch = 31;
int backActuator = 32;
int backActuatorSwitch = 33;
int maxPosVolt = 255;

//Pump Control
int currentSpeed = 51;
int pumpSwitch = 45;
int pumpSpeedSwitch = 44;
int maxSpeedVoltage = 5;
int minSpeedVOltage = 0;

// Barometric Pressure
float initDepth;
float currentDepth;

// I/O variables
int inputDepthFromAndroid;
String data[7];
String outputData;

//+/- Degrees of tilt till auto stabalizing turns on
int stabalizingThreshold = 5;

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

Quaternion q;           // [w, x, y, z]         quaternion container
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector


/*  INTERRUPT DETECTION ROUTINE    */
volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
  mpuInterrupt = true;
}


void setup()
{
  Wire.begin();

  //set up serial communications
  Serial.begin(115200);

  //Start the Barometric Pressure Sensor & Set Initial depth
  bmp.begin();
  initDepth = bmp.readAltitude();
  prevTime = millis();

  //Accelerometer/Gyroscope Setup ----------------------------------------
  Serial.println("Initializing I2C devices...");
  mpu.initialize();

  // verify connection
  Serial.println("Testing device connections...");
  Serial.println(mpu.testConnection() ? ("MPU6050 connection successful") : ("MPU6050 connection failed"));

  // wait for ready
  /*Serial.println("\nSend any character to begin DMP programming and demo: ");
   while (Serial.available() && Serial.read()); // empty buffer
   while (!Serial.available());                 // wait for data
   while (Serial.available() && Serial.read()); // empty buffer again*/

  // load and configure the DMP
  Serial.println("Initializing DMP...");
  devStatus = mpu.dmpInitialize();

  if (devStatus == 0) {
    // turn on the DMP, now that it's ready
    Serial.println("Enabling DMP...");
    mpu.setDMPEnabled(true);

    // enable Arduino interrupt detection
    Serial.println("Enabling interrupt detection (Arduino external interrupt 0)...");
    attachInterrupt(0, dmpDataReady, RISING);
    mpuIntStatus = mpu.getIntStatus();

    // set our DMP Ready flag so the main loop() function knows it's okay to use it
    Serial.println("DMP ready! Waiting for first interrupt...");
    dmpReady = true;

    // get expected DMP packet size for later comparison
    packetSize = mpu.dmpGetFIFOPacketSize();
  } 
  else {
    // ERROR!
    // 1 = initial memory load failed
    // 2 = DMP configuration updates failed
    // (if it's going to break, usually the code will be 1)
    Serial.print("DMP Initialization failed (code ");
    Serial.print(devStatus);
    Serial.println(")");
  }

  //--------------------------------------------------------------------------



  //set up global recvChar to indicate nothing received
  recvChar = NULL;
  WiFi.init();
  sms[0]='a';
  sms[1]='b';
  sms[2]='\0';
  outBuffer[0] = '\0';

  //Enable Pins for Use
  pinMode(ledPin, OUTPUT);
  pinMode(frontActuator, OUTPUT);
  pinMode(frontActuatorSwitch, OUTPUT);
  pinMode(backActuator, OUTPUT);
  pinMode(backActuatorSwitch, OUTPUT);
  pinMode(pumpSwitch, OUTPUT);
  pinMode(pumpSpeedSwitch, OUTPUT);


}

float meterToInch(float meters) {
  return (meters*39.071);
}

int calcDepthVolt(int inches) {
  return ceil((inches*maxPosVolt)/36);
}

void autoDepthControl() {
  sms[0]='0';
  inputDepthFromAndroid = atoi(sms);
  Serial.println(inputDepthFromAndroid);    
  //digitalWrite(frontActuatorSwitch, HIGH);
  //digitalWrite(backActuatorSwitch, HIGH);
  //analogWrite(frontActuator,calcDepthVolt(inputDepthFromAndroid));
  //analogWrite(backActuator,calcDepthVolt(inputDepthFromAndroid));
}

void changeSpeed() {
  sms[0]='0';
  currentSpeed = 255*((atof(sms)/5));
  Serial.println(currentSpeed);    
}

void gyroStabalizing() {

}

void sendData() {
  // Send Back Data every 'interval' milliseconds
  if(millis() - prevTime > interval) {
    prevTime = millis();

    // Depth Measurements		
    dtostrf(currentDepth, 3, 2, charBuf);
    data[0] = String(charBuf);

    //Accel/Gyro
    dtostrf((ypr[0] * 180/M_PI), 4, 2, charBuf);
    data[1] = String(charBuf);
    dtostrf((ypr[1] * 180/M_PI), 4, 2, charBuf);
    data[2] = String(charBuf);
    dtostrf((ypr[2] * 180/M_PI), 4, 2, charBuf);
    data[3] = String(charBuf);
    dtostrf(aaWorld.x, 4, 2, charBuf);
    data[4] = String(charBuf);
    dtostrf(aaWorld.y, 4, 2, charBuf);
    data[5] = String(charBuf);
    dtostrf(aaWorld.z, 4, 2, charBuf);
    data[6] = String(charBuf);

    // Large string of data to send to Android (delimited by ',')
    outputData = data[0] + "," + data[1] + "," + data[2] + "," + data[3] + "," + data[4] + "," + data[5] + "," + data[6];
    Serial.println(outputData);
    outputData.toCharArray(outBuffer, 40);
    //memcpy (outBuffer,outputData,strlen(outputData)+1);
  }
}

void loop()
{
  WiFi.run();
  currentDepth = bmp.readAltitude() - initDepth;

  //while (!mpuInterrupt && fifoCount < packetSize) {
  if(NULL != recvChar) {

    if(sms[0]=='0'){
      digitalWrite(ledPin,LOW);  	
    }
    else if(sms[0]=='1'){
      digitalWrite(ledPin,HIGH);
    }

    /* RISE BUTTON */
    if(sms[0]=='2'){
      digitalWrite(risePin,HIGH); 
      digitalWrite(frontActuatorSwitch, HIGH);
      digitalWrite(backActuatorSwitch, HIGH);
      digitalWrite(frontActuator, HIGH);      
      digitalWrite(backActuator, HIGH);		

    }
    else if(sms[0]=='6'){
      digitalWrite(risePin,LOW);
      digitalWrite(frontActuatorSwitch, LOW);
      digitalWrite(backActuatorSwitch, LOW);      
    }

    /* DIVE BUTTON */
    if(sms[0]=='3'){
      digitalWrite(fallPin,HIGH);   
      digitalWrite(frontActuatorSwitch, HIGH);
      digitalWrite(backActuatorSwitch, HIGH);   
      digitalWrite(frontActuator, HIGH);
      digitalWrite(backActuator, HIGH);
      delay(300);
      digitalWrite(frontActuator, LOW);
      digitalWrite(backActuator, LOW);         

    } 
    else if(sms[0]=='7'){
      digitalWrite(fallPin,LOW);
      digitalWrite(frontActuatorSwitch, LOW);
      digitalWrite(backActuatorSwitch, LOW);   
    }

    /* FORWARD BUTTON */
    if(sms[0]=='4'){
      digitalWrite(forwardPin,HIGH);
      analogWrite(pumpSpeedSwitch, currentSpeed);
      digitalWrite(pumpSwitch, HIGH);       
    }
    else if(sms[0]=='8'){
      digitalWrite(forwardPin,LOW);
      digitalWrite(pumpSwitch, LOW); 
    }

    /* BACKWARD BUTTON */
    if(sms[0]=='5'){
      digitalWrite(backwardPin,HIGH);
      analogWrite(pumpSpeedSwitch, currentSpeed);
      digitalWrite(pumpSwitch, HIGH); 
      // Enable Reverse Mechanism
    } 
    else if(sms[0]=='9'){
      digitalWrite(backwardPin,LOW);
      digitalWrite(pumpSwitch, LOW); 
    } 

    /* INPUT DEPTH BUTTON */
    if(sms[0]=='d'){
      autoDepthControl();

    } 

    /* SPEED SLIDER */
    if(sms[0]=='s'){
      changeSpeed();
    } 


  }
  // }

  mpuInterrupt = false;
  mpuIntStatus = mpu.getIntStatus();
  fifoCount = mpu.getFIFOCount();

  if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
    // reset so we can continue cleanly
    mpu.resetFIFO();
    Serial.println(F("FIFO overflow!"));

    // otherwise, check for DMP data ready interrupt (this should happen frequently)
  } 
  else if (mpuIntStatus & 0x02) {
    // wait for correct available data length, should be a VERY short wait
    while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

    // read a packet from FIFO
    mpu.getFIFOBytes(fifoBuffer, packetSize);

    // track FIFO count here in case there is > 1 packet available
    // (this lets us immediately read more without waiting for an interrupt)
    fifoCount -= packetSize;

    // Yaw/Pitch/Roll
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
    /* Serial.print("ypr\t");
     Serial.print(ypr[0] * 180/M_PI);
     Serial.print("\t");
     Serial.print(ypr[1] * 180/M_PI);
     Serial.print("\t");
     Serial.println(ypr[2] * 180/M_PI);*/

    // Real World Acceleration
    mpu.dmpGetAccel(&aa, fifoBuffer);
    mpu.dmpGetLinearAccelInWorld(&aaWorld, &aaReal, &q);
    /*Serial.print("aworld\t");
     Serial.print(aaWorld.x);
     Serial.print("\t");
     Serial.print(aaWorld.y);
     Serial.print("\t");
     Serial.println(aaWorld.z);*/

  }    



  //gyroStabalizing();
  //sendData();

}


