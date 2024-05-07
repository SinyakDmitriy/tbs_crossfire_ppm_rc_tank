/*
  RC Remote Car
  fsi6x-RC-car-spin.ino
  Uses Flysky FS-I6X RC receiver & FS-I6X 6-ch Controller
  Uses TB6612FNG H-Bridge Motor Controller
  Drives two DC Motors

  Two Drive Modes - Normal and Spin
  Map channel 6 on controller to switch SWA for mode control

  Right stick controls direction in Normal mode (CH1 & CH2)
  VRA controls speed and direction in Spin mode (CH5)
  Left stick is acceleration in both modes (CH3)

  Channel functions by Ricardo Paiva - https://gist.github.com/werneckpaiva/

  DroneBot Workshop 2021
  https://dronebotworkshop.com
*/

#include "ppm.h"

// PPM channel layout (update for your situation)
#define THROTTLE        1
#define ROLL            2
#define PITCH           3
#define YAW             4
#define SWITCH3WAY_1    5
#define BUTTON          6
#define SWITCH3WAY_2    7     // trim-pot for left/right motor mix  (face trim)
#define POT             8     // trim-pot on the (front left edge trim)
// Channel Values

int rcCH1 = 0; // Left - Right
int rcCH2 = 0; // Forward - Reverse
int rcCH3 = 0; // Acceleration
int rcCH5 = 0; // Spin Control
bool rcCH6 = 0; // Mode Control

int throttle      =  0;
int roll          =  0;
int pitch         =   0;
int yaw           =  0;

// LED Connection
#define carLED 13

// Motor A Control Connections
#define pwmA 3
#define in1A 5
#define in2A 4

// Motor B Control Connections
#define pwmB 9
#define in1B 7
#define in2B 8

// TB6612FNG Standby Pin
#define stby 6

// Motor Speed Values - Start at zero
int MotorSpeedA = 0;
int MotorSpeedB = 0;

// Motor Direction Values - 0 = backward, 1 = forward
int MotorDirA = 1;
int MotorDirB = 1;

// Control Motor A
void mControlA(int mspeed, int mdir) {

  // Determine direction
  if (mdir == 0) {
    // Motor backward
    digitalWrite(in1A, LOW);
    digitalWrite(in2A, HIGH);
  } else {
    // Motor forward
    digitalWrite(in1A, HIGH);
    digitalWrite(in2A, LOW);
  }

  // Control motor
  analogWrite(pwmA, mspeed);

}

// Control Motor B
void mControlB(int mspeed, int mdir) {

  // Determine direction
  if (mdir == 0) {
    // Motor backward
    digitalWrite(in1B, LOW);
    digitalWrite(in2B, HIGH);
  } else {
    // Motor forward
    digitalWrite(in1B, HIGH);
    digitalWrite(in2B, LOW);
  }

  // Control motor
  analogWrite(pwmB, mspeed);

}

// Read the number of a given channel and convert to the range provided.
// If the channel is off, return the default value
int readChannel(int channelInput, int minLimit, int maxLimit, int defaultValue) {
  uint16_t ch =  ppm.read_channel(channelInput);
  if (ch < 100) return defaultValue;
  return map(ch, 900, 2100, minLimit, maxLimit);
}

void setup(){

  // Start serial monitor for debugging
  Serial.begin(115200);
  ppm.begin(2, true);

  // Attach iBus object to serial port
  // ibus.begin(Serial);

  // Set all the motor control pins to outputs

  pinMode(pwmA, OUTPUT);
  pinMode(pwmB, OUTPUT);
  pinMode(in1A, OUTPUT);
  pinMode(in2A, OUTPUT);
  pinMode(in1B, OUTPUT);
  pinMode(in2B, OUTPUT);
  pinMode(stby, OUTPUT);

  // Set LED pin as output
  pinMode(carLED, OUTPUT);

  // Keep motors on standby for two seconds & flash LED
  digitalWrite(stby, LOW);
  digitalWrite(carLED, HIGH);
  delay (1000);
  digitalWrite(carLED, LOW);
  delay (1000);
  digitalWrite(stby, HIGH);

}

void loop() {

  // Get RC channel values
  throttle      =   readChannel(THROTTLE, -100, 100, 0);
  pitch         =   readChannel(PITCH, -100, 100, 0);

  // Set speeds with channel 3 value
  MotorSpeedA = throttle;
  MotorSpeedB = throttle;

    // Turn off LED
  digitalWrite(carLED, LOW);

  // Set forward/backward direction with channel 2 value
  if (pitch >= 10) {
    //Forward
    MotorDirA = 1;
    MotorDirB = 1;
    Serial.println("Forward");
  } else {
    //Backward
    MotorDirA = 0;
    MotorDirB = 0;
    Serial.println("Backward");
  }

  // Add channel 2 speed
  MotorSpeedA = MotorSpeedA + abs(pitch);
  MotorSpeedB = MotorSpeedB + abs(pitch);

  // Set left/right offset with channel 1 value
  MotorSpeedA = MotorSpeedA - throttle;
  MotorSpeedB = MotorSpeedB + throttle;

  

  // Ensure that speeds are between 0 and 255
  MotorSpeedA = constrain(MotorSpeedA, 0, 255);
  MotorSpeedB = constrain(MotorSpeedB, 0, 255);

  //Drive Motors
  mControlA(MotorSpeedA, MotorDirA);
  mControlB(MotorSpeedB, MotorDirB);

  // Print speed values to serial monitor for debugging
  Serial.print("Motor A Speed = ");
  Serial.print(MotorSpeedA);
  Serial.print(" | Motor B Speed = ");
  Serial.println(MotorSpeedB);

  // Slight delay
  delay(50);

}
