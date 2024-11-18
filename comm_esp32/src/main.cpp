#include <Arduino.h>
#include <Stepper.h>  // Include the Stepper library to control the translation stage

// Define the number of steps for each motor 
#define STEPS 200

// Define pins for each stepper motor 
#define X_STEP_PIN_1 15 // A+
#define X_STEP_PIN_2 17 // B+
#define X_STEP_PIN_3 14 // A-
#define X_STEP_PIN_4 4  // B-

#define Y_STEP_PIN_1 22 // A+
#define Y_STEP_PIN_2 21 // B+
#define Y_STEP_PIN_3 19 // A-
#define Y_STEP_PIN_4 18 // B-

// Create instances of the Stepper class for X and Y axes
Stepper stepperX(STEPS, X_STEP_PIN_1, X_STEP_PIN_2, X_STEP_PIN_3, X_STEP_PIN_4);
Stepper stepperY(STEPS, Y_STEP_PIN_1, Y_STEP_PIN_2, Y_STEP_PIN_3, Y_STEP_PIN_4);

// Variables to store current positions
int currentX = 0;
int currentY = 0;

// Target positions received from the serial data
int targetX = 0;
int targetY = 0;

// Calibration variables (you might need to adjust these based on your setup)
int centerX = 640;  // Example center X value (e.g., 1280 / 2 for 1280px image)
int centerY = 512;  // Example center Y value (e.g., 1024 / 2 for 1024px image)

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect
  }
  Serial.println("ESP32 is ready to receive data!");

  // Set the speed of the motors 
  stepperX.setSpeed(60);
  stepperY.setSpeed(60);
}

void loop() {
  // Check for incoming data
  if (Serial.available() > 0) {
    // Read the incoming data until a newline character
    String data = Serial.readStringUntil('\n');
    
    // Debug print for incoming data
    Serial.print("Received raw data: ");
    Serial.println(data);
    
    // Parse the x and y values from the data
    int commaIndex = data.indexOf(',');
    if (commaIndex > 0) {
      targetX = data.substring(0, commaIndex).toInt();
      targetY = data.substring(commaIndex + 1).toInt();

      // Debug print the parsed target position
      Serial.print("Parsed Target X position: ");
      Serial.println(targetX);
      Serial.print("Parsed Target Y position: ");
      Serial.println(targetY);

      // Calculate the offset from the center (i.e., how far we need to move the motors)
      int offsetX = targetX - centerX;
      int offsetY = targetY - centerY;

      // Debug print the offsets
      Serial.print("Offset X: ");
      Serial.println(offsetX);
      Serial.print("Offset Y: ");
      Serial.println(offsetY);

      // Move the motors to the target position
      moveMotors(offsetX, offsetY);
    } else {
      // If there is no comma, print an error message
      Serial.println("Error: Invalid data format. No comma found.");
    }
  }

  // Small delay to control speed and avoid overloading the loop
  delay(10);
}

void moveMotors(int offsetX, int offsetY) {
  // Calculate the number of steps needed to reach the target position for each axis
  int stepsX = offsetX - currentX;
  int stepsY = offsetY - currentY;

  // Move the X axis stepper to the target position
  if (stepsX != 0) {
    stepperX.step(stepsX > 0 ? 1 : -1); // Move one step in the direction of the target
    currentX += (stepsX > 0 ? 1 : -1);  // Update the current position
    Serial.print("Moving X to: ");
    Serial.println(currentX);  // Print current X position after moving
  }

  // Move the Y axis stepper to the target position
  if (stepsY != 0) {
    stepperY.step(stepsY > 0 ? 1 : -1); // Move one step in the direction of the target
    currentY += (stepsY > 0 ? 1 : -1);  // Update the current position
    Serial.print("Moving Y to: ");
    Serial.println(currentY);  // Print current Y position after moving
  }

  // Update the current positions after the move
  currentX = offsetX;
  currentY = offsetY;
}
