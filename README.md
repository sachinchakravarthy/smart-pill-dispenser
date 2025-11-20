# 🏥 Smart AI-Powered Pill Dispenser

An IoT-enabled automated medication dispensing system built with ESP32, featuring WiFi web control, scheduled dispensing, and real-time monitoring.

![Project Status](https://img.shields.io/badge/status-active-success.svg)
![License](https://img.shields.io/badge/license-MIT-blue.svg)

## 📋 Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Software Requirements](#software-requirements)
- [Circuit Diagram](#circuit-diagram)
- [Installation](#installation)
- [Usage](#usage)
- [API Documentation](#api-documentation)
- [Project Structure](#project-structure)
- [Screenshots](#screenshots)
- [Future Enhancements](#future-enhancements)
- [Contributing](#contributing)
- [License](#license)
- [Contact](#contact)

## 🎯 Overview

The Smart Pill Dispenser is an automated medication management system designed to help patients adhere to their medication schedules. The system uses a rotating tray mechanism with 6 compartments, controlled by an ESP32 microcontroller, and provides a web-based interface for remote monitoring and control.

### Key Highlights
- **WiFi-enabled remote control** via web interface
- **Automated time-based dispensing** with customizable schedules
- **Real-time inventory tracking** with low-stock alerts
- **Multi-compartment system** for different medications
- **Visual and audio feedback** through LEDs and buzzer
- **Mobile-responsive interface** accessible from any device

## ✨ Features

### Hardware Features
- ✅ 6-compartment rotating tray system
- ✅ Precise stepper motor control for compartment alignment
- ✅ Servo-controlled gate mechanism for pill dispensing
- ✅ LED status indicators (Green/Yellow/Red)
- ✅ Audio feedback via buzzer
- ✅ Physical push-button for manual dispensing
- ✅ Low power consumption

### Software Features
- ✅ WiFi web server with RESTful API
- ✅ Responsive HTML5 web interface
- ✅ Schedule-based automated dispensing
- ✅ Real-time system status monitoring
- ✅ Pill inventory management
- ✅ Multiple schedule support (up to 10)
- ✅ Day-of-week scheduling
- ✅ Automatic low-stock warnings
- ✅ System uptime tracking
- ✅ NTP time synchronization

## 🛠️ Hardware Requirements

### Core Components
| Component | Quantity | Specification | Purpose |
|-----------|----------|---------------|---------|
| ESP32 Dev Module | 1 | ESP32-WROOM-32 or compatible | Main controller with WiFi |
| 28BYJ-48 Stepper Motor | 1 | 5V, 2048 steps/revolution | Rotates compartment tray |
| ULN2003 Driver Board | 1 | 5-12V | Stepper motor driver |
| SG90 Servo Motor | 1 | 5V, 180° rotation | Gate control for dispensing |
| Push Button | 1 | Momentary switch | Manual dispense trigger |
| Buzzer | 1 | 5V active buzzer | Audio feedback |
| LEDs | 3 | 5mm (Green, Yellow, Red) | Status indicators |
| Resistors | 4 | 220Ω (LEDs), 100Ω (Buzzer) | Current limiting |
| Power Bank | 1 | 5V USB output, 2A+ | Portable power source |
| Jumper Wires | ~20 | Male-to-male, male-to-female | Connections |
| Breadboard | 1 | Standard size | Component assembly |
| Cardboard/3D Printed Parts | - | - | Mechanical structure |

### Optional Components
- HX711 Load Cell Module (for weight-based pill detection)
- 3D-printed enclosure
- Rechargeable battery pack

## 💻 Software Requirements

### Development Tools
- **Arduino IDE** (v2.0+) or **PlatformIO**
- **USB Cable** (Micro-USB or USB-C, data-capable)
- **Serial Monitor** (built into Arduino IDE)

### Required Libraries
Install via Arduino IDE Library Manager:
```
- WiFi (built-in ESP32 core)
- WebServer (built-in ESP32 core)
- Stepper (Arduino built-in)
- ESP32Servo by Kevin Harrington
- ArduinoJson by Benoit Blanchon (v6.x)
```

### ESP32 Board Support
Add ESP32 board manager URL in Arduino IDE:
```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

## 🔌 Circuit Diagram

### Pin Connections

#### Stepper Motor (via ULN2003)
```
ULN2003 Pin → ESP32 GPIO
IN1         → GPIO 18
IN2         → GPIO 19
IN3         → GPIO 21
IN4         → GPIO 22
VCC (+)     → 5V (VIN)
GND (-)     → GND
Motor       → 5-pin connector on ULN2003
```

#### Servo Motor
```
Servo Pin   → ESP32 Pin
Signal (Orange) → GPIO 13
VCC (Red)       → 5V (VIN)
GND (Brown)     → GND
```

#### Control & Feedback
```
Component   → ESP32 Pin
Push Button → GPIO 14 (other leg to GND)
Buzzer (+)  → GPIO 25 (via 100Ω resistor)
Buzzer (-)  → GND
Green LED   → GPIO 26 (via 220Ω resistor)
Yellow LED  → GPIO 27 (via 220Ω resistor)
Red LED     → GPIO 32 (via 220Ω resistor)
All LED (-) → GND
```

#### Power
```
Power Bank USB → ESP32 Micro-USB
ESP32 5V (VIN) → ULN2003 VCC, Servo VCC
ESP32 GND      → All component grounds (common ground)
```

### Schematic
```
[Power Bank] 
     |
     USB
     |
 [ESP32 Dev]
     |
     ├── GPIO 18,19,21,22 → [ULN2003] → [Stepper Motor]
     ├── GPIO 13 → [Servo Motor]
     ├── GPIO 14 ← [Push Button] → GND
     ├── GPIO 25 → [Buzzer] → GND
     ├── GPIO 26 → [Green LED] → GND
     ├── GPIO 27 → [Yellow LED] → GND
     └── GPIO 32 → [Red LED] → GND
```

## 📥 Installation

### Step 1: Clone the Repository
```bash
git clone https://github.com/YOUR_USERNAME/smart-pill-dispenser.git
cd smart-pill-dispenser
```

### Step 2: Hardware Assembly
1. Connect all components according to the circuit diagram
2. Ensure all grounds are connected together (common ground)
3. Double-check power connections before applying power
4. Test stepper and servo motors individually first

### Step 3: Software Setup

#### Install Arduino IDE
1. Download Arduino IDE from [arduino.cc](https://www.arduino.cc/en/software)
2. Install the IDE for your operating system

#### Add ESP32 Board Support
1. Open Arduino IDE
2. Go to `File → Preferences`
3. Add ESP32 board manager URL (see Software Requirements)
4. Go to `Tools → Board → Boards Manager`
5. Search "ESP32" and install "esp32 by Espressif Systems"

#### Install Required Libraries
1. Go to `Sketch → Include Library → Manage Libraries`
2. Install each library listed in Software Requirements

### Step 4: Configure WiFi
1. Open `esp32-wifi-dispenser.ino` in Arduino IDE
2. Find these lines and update with your WiFi credentials:
```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```

### Step 5: Upload Code
1. Connect ESP32 to computer via USB
2. Select board: `Tools → Board → ESP32 Arduino → ESP32 Dev Module`
3. Select port: `Tools → Port → COM X` (Windows) or `/dev/ttyUSBX` (Linux)
4. Click **Upload** (or press `Ctrl+U`)
5. Hold **BOOT** button on ESP32 if upload fails

### Step 6: Get Device IP Address
1. Open Serial Monitor (`Tools → Serial Monitor`)
2. Set baud rate to `115200`
3. Press **EN** (reset) button on ESP32
4. Note the IP address displayed (e.g., `192.168.1.100`)

## 🚀 Usage

### Access Web Interface
1. Ensure your device (phone/computer) is on the same WiFi network
2. Open web browser
3. Navigate to: `http://[ESP32_IP_ADDRESS]`
4. You'll see the control dashboard

### Manual Dispensing
1. Select compartment from dropdown (0-5)
2. Click **"💊 Dispense Now"**
3. System rotates to compartment and dispenses pill
4. Pill count automatically decrements

### Schedule Setup
1. Go to **"📅 Add Schedule"** panel
2. Select time (HH:MM format)
3. Choose compartment
4. Select days of week (click to toggle)
5. Click **"➕ Add Schedule"**
6. Pills will dispense automatically at scheduled times

### Inventory Management
1. View current pill counts for all compartments
2. Update counts manually via input fields
3. Click **"🔄 Refill"** to reset to maximum (30 pills)
4. Low stock warnings appear when ≤5 pills remain

### Physical Button
- Press the physical button on device for immediate dispensing from current compartment

## 📡 API Documentation

### Endpoints

#### GET /api/status
Returns system status and compartment information
```json
{
  "currentCompartment": 0,
  "totalDispensedToday": 3,
  "uptime": "2d 5h 30m",
  "compartments": [
    {
      "id": 0,
      "name": "Morning Vitamin",
      "count": 25
    }
  ]
}
```

#### GET /api/dispense?compartment=N
Dispenses pill from specified compartment (0-5)

#### GET /api/rotate?compartment=N
Rotates tray to specified compartment without dispensing

#### GET /api/setcount?compartment=N&count=X
Updates pill count for compartment

#### GET /api/refill?compartment=N
Refills compartment to maximum capacity (30 pills)

#### GET /api/schedules
Returns list of all scheduled dispense times

#### GET /api/addschedule?hour=H&minute=M&compartment=N&days=DDDDDDD
Adds new schedule (days: 7-digit binary string, Mon-Sun)

#### GET /api/delschedule?id=N
Deletes schedule by ID

## 📁 Project Structure

```
smart-pill-dispenser/
│
├── esp32-wifi-dispenser.ino    # Main Arduino sketch
├── README.md                    # This file
├── LICENSE                      # MIT License
│
├── docs/                        # Documentation
│   ├── circuit-diagram.png      # Wiring schematic
│   ├── assembly-guide.md        # Step-by-step assembly
│   └── troubleshooting.md       # Common issues and fixes
│
├── hardware/                    # Hardware files
│   ├── 3d-models/               # STL files for 3D printing
│   │   ├── tray.stl
│   │   ├── enclosure.stl
│   │   └── servo-mount.stl
│   └── bom.csv                  # Bill of materials
│
├── images/                      # Project images
│   ├── prototype.jpg
│   ├── web-interface.png
│   └── demo.gif
│
└── tests/                       # Test sketches
    ├── stepper-test.ino
    ├── servo-test.ino
    └── led-test.ino
```



## 🔮 Future Enhancements

- [ ] **Mobile App** - Native Android/iOS app
- [ ] **Cloud Integration** - Firebase/AWS for remote monitoring
- [ ] **Weight Sensors** - HX711 load cells for automatic pill counting
- [ ] **Voice Control** - Alexa/Google Home integration
- [ ] **User Authentication** - Multi-user support with login
- [ ] **Medication Database** - Drug interaction warnings
- [ ] **Refill Reminders** - SMS/Email notifications
- [ ] **Usage Analytics** - Historical data and graphs
- [ ] **Battery Monitoring** - Low battery alerts
- [ ] **OTA Updates** - Over-the-air firmware updates

## 🤝 Contributing

Contributions are welcome! Please follow these steps:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

### Guidelines
- Follow existing code style
- Add comments for complex logic
- Test thoroughly before submitting
- Update README if adding features

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👤 Contact

**Your Name** - [your.email@example.com](mailto:your.email@example.com)

**Project Link:** [https://github.com/YOUR_USERNAME/smart-pill-dispenser](https://github.com/YOUR_USERNAME/smart-pill-dispenser)

**LinkedIn:** [Your LinkedIn Profile](https://linkedin.com/in/your-profile)

---

## 🙏 Acknowledgments

- ESP32 Arduino Core by Espressif Systems
- ArduinoJson library by Benoit Blanchon
- ESP32Servo library by Kevin Harrington
- Inspiration from various IoT healthcare projects

## ⚠️ Disclaimer

This project is for educational and demonstration purposes. It is not a medical device and should not be used as a replacement for professional medical advice or prescription medication management systems. Always consult healthcare professionals for medical guidance.

---

**Built with ❤️ using ESP32 and Arduino**

*If you found this project helpful, please consider giving it a ⭐ on GitHub!*
