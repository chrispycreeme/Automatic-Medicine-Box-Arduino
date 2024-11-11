#include <Servo.h>

// Create servo objects
Servo servoA;
Servo servoB;
Servo servoC;
Servo servoD;

// Define servo pins
const int SERVO_A_PIN = 13;
const int SERVO_B_PIN = 9;
const int SERVO_C_PIN = 11;
const int SERVO_D_PIN = 10;

// Custom constants for each servo
struct ServoConstants {
    int openAngle;      // Angle to open container
    int closedAngle;    // Angle to close container
    int openTime;       // Time to keep container open (milliseconds)
    int delayAfter;     // Delay after closing (milliseconds)
    const char* name;   // Name for debugging
};

// Define specific constants for each servo/container
const ServoConstants CONTAINER_A = {
    .openAngle = 40,    
    .closedAngle = 0,   
    .openTime = 100,    // Faster opening time
    .delayAfter = 300,  // Short delay after
    .name = "A"
};

const ServoConstants CONTAINER_B = {
    .openAngle = 60,    // Slightly different angle
    .closedAngle = 0,    // Not fully closed
    .openTime = 120,    // Longer opening time
    .delayAfter = 500,   
    .name = "B"
};

const ServoConstants CONTAINER_C = {
    .openAngle = 60,    
    .closedAngle = 0,   
    .openTime = 200,    
    .delayAfter = 400,   
    .name = "C"
};

const ServoConstants CONTAINER_D = {
    .openAngle = 70,    
    .closedAngle = 0,   
    .openTime = 200,    // Longest opening time
    .delayAfter = 600,   // Longer delay after
    .name = "D"
};

void setup() {
    // Initialize Serial for debugging
    Serial.begin(115200);
    Serial.println("Custom Servo Constants Test Starting...");
    
    // Attach servos to pins
    servoA.attach(SERVO_A_PIN);
    servoB.attach(SERVO_B_PIN);
    servoC.attach(SERVO_C_PIN);
    servoD.attach(SERVO_D_PIN);
    
    // Initialize all servos to closed position
    resetServos();
    
    // Wait for everything to stabilize
    delay(1000);
}

void loop() {
    Serial.println("\nStarting dispense sequence...");
    
    // Dispense from each container with its custom constants
    dispenseFromContainer(servoA, CONTAINER_A);
    dispenseFromContainer(servoB, CONTAINER_B);
    dispenseFromContainer(servoC, CONTAINER_C);
    dispenseFromContainer(servoD, CONTAINER_D);
    
    // Wait before starting next cycle
    Serial.println("Sequence complete. Waiting 3 seconds...");
    delay(3000);
}

void dispenseFromContainer(Servo &servo, const ServoConstants &constants) {
    // Print start message
    Serial.print("Opening container ");
    Serial.print(constants.name);
    Serial.print(" to angle ");
    Serial.println(constants.openAngle);
    
    // Open container
    servo.write(constants.openAngle);
    delay(constants.openTime);
    
    // Close container
    Serial.print("Closing container ");
    Serial.print(constants.name);
    Serial.print(" to angle ");
    Serial.println(constants.closedAngle);
    servo.write(constants.closedAngle);
    
    // Wait specified delay time
    delay(constants.delayAfter);
}

void resetServos() {
    servoA.write(CONTAINER_A.closedAngle);
    servoB.write(CONTAINER_B.closedAngle);
    servoC.write(CONTAINER_C.closedAngle);
    servoD.write(CONTAINER_D.closedAngle);
}