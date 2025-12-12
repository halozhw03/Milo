// ============================================================================
// Milo Robot - Integrated Control System
// ============================================================================
// This file integrates all four original programs:
// - Bluetooth control mode (default)
// - Auto-follow mode (ultrasonic sensor)
// - Test mode (forward/backward and circle tests)
//
// Mode switching via Serial commands: "auto", "bluetooth", "test"
// ============================================================================

// Motor control pins (L298N driver)
const int IN1 = 8;  // Left motor direction pin 1
const int IN2 = 9;  // Left motor direction pin 2
const int IN3 = 10; // Right motor direction pin 1
const int IN4 = 11; // Right motor direction pin 2
const int ENA = 5;  // Left motor speed control (PWM)
const int ENB = 6;  // Right motor speed control (PWM)

// Ultrasonic sensor pin
const int pingPin = 7;  // PING))) ultrasonic sensor pin

// Speed and timing constants
const int maxSpeed = 180;           // Maximum motor speed (0-255)
const int autoFollowSpeed = 160;    // Speed for auto-follow mode
const int testSpeed = 100;          // Speed for test mode
const unsigned long bluetoothTimeout = 300;  // Bluetooth command timeout (ms)
const unsigned long autoFollowDelay = 200;  // Auto-follow measurement interval (ms)

// Mode enumeration
enum RobotMode {
  MODE_BLUETOOTH,    // Bluetooth control mode (default)
  MODE_AUTO_FOLLOW,  // Auto-follow mode using ultrasonic sensor
  MODE_TEST          // Test mode for debugging
};

RobotMode currentMode = MODE_BLUETOOTH;  // Default mode

// Bluetooth control variables
unsigned long lastCommandTime = 0;
String inputLine = "";

// Test mode variables
unsigned long testStartTime = 0;
int testPhase = 0;  // 0: forward/backward test, 1: circle test
bool testRunning = false;

// ============================================================================
// Setup Function
// ============================================================================
void setup() {
  // Initialize motor control pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  
  // Initialize ultrasonic sensor pin
  pinMode(pingPin, OUTPUT);
  
  // Initialize serial communication
  Serial.begin(9600);   // Debug serial (for mode switching commands)
  Serial1.begin(9600);  // Bluetooth serial (HC-10)
  
  // Initial message
  Serial.println("Milo Robot - Integrated Control System");
  Serial.println("Mode: BLUETOOTH (default)");
  Serial.println("Commands: 'auto', 'bluetooth', 'test'");
  
  // Stop motors initially
  stopMoving();
}

// ============================================================================
// Main Loop
// ============================================================================
void loop() {
  // Check for mode switching commands from Serial
  checkModeSwitch();
  
  // Execute current mode
  switch (currentMode) {
    case MODE_BLUETOOTH:
      processBluetoothMode();
      break;
    case MODE_AUTO_FOLLOW:
      processAutoFollowMode();
      break;
    case MODE_TEST:
      processTestMode();
      break;
  }
}

// ============================================================================
// Mode Switching Functions
// ============================================================================
void checkModeSwitch() {
  while (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toLowerCase();
    
    if (command == "auto") {
      switchMode(MODE_AUTO_FOLLOW);
    } else if (command == "bluetooth") {
      switchMode(MODE_BLUETOOTH);
    } else if (command == "test") {
      switchMode(MODE_TEST);
    }
  }
}

void switchMode(RobotMode newMode) {
  if (currentMode != newMode) {
    stopMoving();  // Stop motors when switching modes
    currentMode = newMode;
    
    // Reset mode-specific variables
    inputLine = "";
    testRunning = false;
    testPhase = 0;
    
    // Print mode confirmation
    Serial.print("Mode switched to: ");
    switch (newMode) {
      case MODE_BLUETOOTH:
        Serial.println("BLUETOOTH");
        break;
      case MODE_AUTO_FOLLOW:
        Serial.println("AUTO_FOLLOW");
        break;
      case MODE_TEST:
        Serial.println("TEST");
        testStartTime = millis();
        testRunning = true;
        break;
    }
  }
}

// ============================================================================
// Bluetooth Control Mode
// ============================================================================
void processBluetoothMode() {
  // Read data from Bluetooth serial
  while (Serial1.available()) {
    char c = Serial1.read();
    if (c == '\n' || c == '\r') {
      processJoystickCommand(inputLine);
      inputLine = "";
    } else {
      inputLine += c;
    }
  }
  
  // Auto-stop if no command received within timeout
  if (millis() - lastCommandTime > bluetoothTimeout) {
    stopMoving();
  }
}

void processJoystickCommand(String cmd) {
  // Check if command starts with joystick prefix
  if (!cmd.startsWith("J0:")) return;
  
  // Parse rotation and magnitude
  int commaIndex = cmd.indexOf(',');
  if (commaIndex == -1) return;
  
  float rotation = cmd.substring(3, commaIndex).toFloat();
  float magnitude = cmd.substring(commaIndex + 1).toFloat();
  
  Serial.print("Angle = ");
  Serial.print(rotation);
  Serial.print("°, Magnitude = ");
  Serial.println(magnitude);
  
  lastCommandTime = millis();
  controlMotors(rotation, magnitude);
}

void controlMotors(float rot, float mag) {
  // Stop if magnitude is too small
  if (mag < 0.15) {
    stopMoving();
    return;
  }
  
  int speed = maxSpeed * mag;
  
  // Direction control based on rotation angle
  // More tolerant angle ranges for better sensitivity
  if ((rot >= 330 || rot < 30)) {
    turnRight(speed);
  } else if (rot >= 30 && rot < 150) {
    moveBackward(speed);
  } else if (rot >= 150 && rot < 210) {
    turnLeft(speed);
  } else if (rot >= 210 && rot < 330) {
    moveForward(speed);
  }
}

// ============================================================================
// Auto-Follow Mode
// ============================================================================
void processAutoFollowMode() {
  long distance = getDistanceCM(pingPin);
  
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  
  // Control logic based on distance
  if (distance > 60) {
    // Too far, move forward
    moveForward(autoFollowSpeed);
    Serial.println("Moving forward");
  } else if (distance < 25) {
    // Too close, stop
    stopMoving();
    Serial.println("Stopped (too close)");
  } else {
    // Maintain distance (25-60cm), stop
    stopMoving();
    Serial.println("Stopped (maintaining distance)");
  }
  
  delay(autoFollowDelay);  // Wait before next measurement
}

// ============================================================================
// Test Mode
// ============================================================================
void processTestMode() {
  if (!testRunning) {
    testStartTime = millis();
    testRunning = true;
    testPhase = 0;
  }
  
  unsigned long elapsed = millis() - testStartTime;
  
  // Phase 0: Forward/Backward Test
  if (testPhase == 0) {
    if (elapsed < 2000) {
      // Forward for 2 seconds
      moveForward(testSpeed);
      Serial.println("Test: Moving forward");
    } else if (elapsed < 3000) {
      // Stop for 1 second
      stopMoving();
      Serial.println("Test: Stopped");
    } else if (elapsed < 5000) {
      // Backward for 2 seconds
      moveBackward(testSpeed);
      Serial.println("Test: Moving backward");
    } else if (elapsed < 6000) {
      // Stop for 1 second
      stopMoving();
      Serial.println("Test: Stopped");
    } else {
      // Move to next phase
      testPhase = 1;
      testStartTime = millis();
    }
  }
  // Phase 1: Circle Test
  else if (testPhase == 1) {
    if (elapsed < 10000) {
      // Rotate in circle for 10 seconds
      turnLeft(testSpeed);
      Serial.println("Test: Rotating (circle)");
    } else if (elapsed < 13000) {
      // Stop for 3 seconds
      stopMoving();
      Serial.println("Test: Stopped");
    } else {
      // Reset to start of test cycle
      testPhase = 0;
      testStartTime = millis();
      Serial.println("Test: Cycle restarted");
    }
  }
}

// ============================================================================
// Motor Control Functions
// ============================================================================
void moveForward(int spd) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, spd);
  analogWrite(ENB, spd);
}

void moveBackward(int spd) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, spd);
  analogWrite(ENB, spd);
}

void turnLeft(int spd) {
  // Left motor forward, right motor backward (rotate in place)
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);  // Right motor backward
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);   // Left motor forward
  analogWrite(ENA, spd);
  analogWrite(ENB, spd);
}

void turnRight(int spd) {
  // Right motor forward, left motor backward (rotate in place)
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);   // Right motor forward
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);  // Left motor backward
  analogWrite(ENA, spd);
  analogWrite(ENB, spd);
}

void stopMoving() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

// ============================================================================
// Ultrasonic Sensor Functions
// ============================================================================
long getDistanceCM(int pin) {
  // Send trigger pulse
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delayMicroseconds(2);
  digitalWrite(pin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pin, LOW);
  
  // Read echo
  pinMode(pin, INPUT);
  long duration = pulseIn(pin, HIGH, 30000);  // Wait up to 30ms
  
  // Timeout means very far away
  if (duration == 0) return 999;
  
  // Convert to centimeters (speed of sound = 340 m/s)
  return duration / 58.0;
}

