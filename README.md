# Milo Robot - Integrated Control System

A multi-mode robot control system based on Arduino, integrating Bluetooth remote control, auto-follow, and test functionality.

## 📋 Project Overview

Milo Robot is a comprehensive robot control system that supports three operation modes:
- **Bluetooth Control Mode**: Remote control via Bluetooth module (HC-10)
- **Auto-Follow Mode**: Automatic following using ultrasonic sensor
- **Test Mode**: Automated forward/backward and circular motion tests

## ✨ Features

- 🔵 **Bluetooth Remote Control**: Joystick control with real-time movement response
- 🤖 **Auto-Follow**: Ultrasonic sensor-based automatic distance maintenance
- 🧪 **Test Mode**: Automated preset action sequences for debugging and verification
- 🔄 **Mode Switching**: Quick mode switching via serial commands
- ⏱️ **Timeout Protection**: Auto-stop when no Bluetooth commands received, ensuring safety

## 🛠️ Hardware Requirements

### Required Components
- Arduino development board (with Serial1 support, e.g., Arduino Mega, Uno, etc.)
- L298N motor driver module
- HC-10 Bluetooth module
- PING))) ultrasonic sensor
- 2 DC motors
- Power supply module (for motors and Arduino)

### Recommended Components
- Robot chassis
- Battery pack
- Jumper wires

## 📌 Pin Configuration

### Motor Control Pins (L298N)
| Function | Arduino Pin | Description |
|----------|-------------|-------------|
| IN1      | 8           | Left motor direction control 1 |
| IN2      | 9           | Left motor direction control 2 |
| IN3      | 10          | Right motor direction control 1 |
| IN4      | 11          | Right motor direction control 2 |
| ENA      | 5           | Left motor speed control (PWM) |
| ENB      | 6           | Right motor speed control (PWM) |

### Sensor Pins
| Function | Arduino Pin | Description |
|----------|-------------|-------------|
| Ultrasonic Sensor | 7 | PING))) sensor signal pin |

### Communication Interfaces
| Function | Arduino Interface | Baud Rate | Description |
|----------|-------------------|-----------|-------------|
| Debug Serial | Serial | 9600 | For mode switching commands and debug info |
| Bluetooth Serial | Serial1 | 9600 | HC-10 Bluetooth module communication |

## 🚀 Quick Start

### 1. Hardware Connection
Connect all hardware components according to the pin configuration above. Ensure:
- Motors are correctly connected to L298N driver module
- Bluetooth module is connected to Serial1 (RX/TX)
- Ultrasonic sensor is connected to pin 7
- All power connections are correct

### 2. Upload Code
1. Open Arduino IDE
2. Select the correct board and port
3. Open `milo_integrated.ino` file
4. Click the "Upload" button

### 3. Use Serial Monitor
1. Open Arduino IDE Serial Monitor (baud rate: 9600)
2. System will display current mode and available commands on startup

## 🎮 Usage

### Mode Switching

Send the following commands via Serial Monitor to switch modes:

| Command | Function | Description |
|---------|----------|-------------|
| `auto` | Switch to Auto-Follow Mode | Robot will use ultrasonic sensor to automatically follow target |
| `bluetooth` | Switch to Bluetooth Control Mode | Default mode, receives control commands via Bluetooth |
| `test` | Switch to Test Mode | Execute automated test sequence |

### Bluetooth Control Mode

#### Control Protocol
Bluetooth module receives commands in the following format:
```
J0:<angle>,<magnitude>
```

- **Angle (rotation)**: 0-360 degrees, indicates movement direction
  - 0°/360°: Turn right
  - 30°-150°: Move backward
  - 150°-210°: Turn left
  - 210°-330°: Move forward

- **Magnitude**: 0.0-1.0, indicates movement speed
  - Less than 0.15: Stop
  - 0.15-1.0: Speed proportional to magnitude

#### Timeout Protection
If no control command is received within 300ms, the robot will automatically stop.

### Auto-Follow Mode

Robot uses ultrasonic sensor to detect distance to objects ahead:

| Distance Range | Action | Description |
|----------------|--------|-------------|
| > 60 cm | Move forward | Too far, move forward |
| 25-60 cm | Stop | Maintain appropriate distance |
| < 25 cm | Stop | Too close, safety stop |

Measurement interval: 200ms

### Test Mode

Automatically executes the following test sequence (looping):

1. **Forward/Backward Test**
   - Move forward for 2 seconds
   - Stop for 1 second
   - Move backward for 2 seconds
   - Stop for 1 second

2. **Circular Motion Test**
   - Turn left for 10 seconds
   - Stop for 3 seconds

After completion, the test automatically restarts the cycle.

## ⚙️ Parameter Configuration

You can modify the following parameters in the code:

```cpp
// Speed parameters
const int maxSpeed = 180;           // Maximum speed (0-255)
const int autoFollowSpeed = 160;    // Auto-follow mode speed
const int testSpeed = 100;          // Test mode speed

// Timing parameters
const unsigned long bluetoothTimeout = 300;  // Bluetooth timeout (milliseconds)
const unsigned long autoFollowDelay = 200;   // Auto-follow measurement interval (milliseconds)

// Distance parameters (Auto-Follow Mode)
if (distance > 60) { ... }          // Too far threshold
else if (distance < 25) { ... }     // Too close threshold
```

## 📊 Serial Output

The system outputs the following information via Serial:

- Startup information: Displays current mode and available commands
- Mode switch confirmation: Prompt when switching modes
- Bluetooth control: Angle and magnitude information
- Auto-follow: Real-time distance measurements
- Test mode: Currently executing test actions

## 🔧 Troubleshooting

### Issue: Motors Not Rotating
- Check L298N driver module power connections
- Verify motor pin connections are correct
- Check if PWM pins (ENA/ENB) are working properly

### Issue: Bluetooth Cannot Connect
- Confirm HC-10 module is correctly connected to Serial1
- Check Bluetooth module baud rate setting (should be 9600)
- Verify Bluetooth module is paired

### Issue: Ultrasonic Sensor Not Responding
- Check sensor pin connection (pin 7)
- Verify sensor power supply is normal
- Check if there are obstacles in front of the sensor

### Issue: Mode Switching Not Responding
- Confirm Serial Monitor baud rate is set to 9600
- Check command input format (lowercase, no extra spaces)
- Verify Serial Monitor is correctly connected

## 📝 Code Structure

```
milo_integrated.ino
├── Pin definitions and constants
├── Mode enumeration and variables
├── setup() - Initialization function
├── loop() - Main loop
├── Mode switching functions
│   ├── checkModeSwitch()
│   └── switchMode()
├── Bluetooth control mode
│   ├── processBluetoothMode()
│   ├── processJoystickCommand()
│   └── controlMotors()
├── Auto-follow mode
│   └── processAutoFollowMode()
├── Test mode
│   └── processTestMode()
├── Motor control functions
│   ├── moveForward()
│   ├── moveBackward()
│   ├── turnLeft()
│   ├── turnRight()
│   └── stopMoving()
└── Sensor functions
    └── getDistanceCM()
```

## 📄 License

This project is open source.

## 👤 Author

Yinhao Zhu

---

**Note**: Please ensure all hardware connections are correct before use, and carefully check power connections to avoid short circuits or overloads.
