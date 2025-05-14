# ESP32 Multi-Mode Robot with PS3 Controller, Line Following, and Color Detection

This project is a multi-functional ESP32-based robot controlled via a PS3 Bluetooth controller and capable of line following, color detection, and manual driving. It utilizes various sensors and peripherals including IR line sensors, a TCS3200 color sensor, DC motors, and a servo for steering.

## Features

* **PS3 Controller Support**: Fully wireless control using a PS3 controller (via Bluetooth).
* **Multiple Operating Modes**:

  * **Line Follower Mode**: Uses 5 IR sensors for line tracking.
  * **Color Detection Mode**: Detects RGB values using the TCS3200 sensor and streams them via Wi-Fi.
  * **Manual Drive Mode**: Control direction and speed using analog sticks.
* **Wi-Fi Communication**:

  * Sends sensor data (e.g., RGB values, IR states) to a LabVIEW dashboard or other TCP clients via two TCP servers.
* **Servo Steering**: Smooth analog steering using a servo motor for direction control.
* **DC Motor Control**: Differential drive with speed control using PWM.

---

## How It Works

### Modes

Modes are selected via **PS3 controller buttons**:

* `SELECT + SQUARE` → Line Follower Mode
* `SELECT + TRIANGLE` → Color Detection Mode
* `SELECT + CIRCLE` → Manual Driving Mode
* `SELECT + START` → Restart / Stop all modes

### PS3 Analog Stick Mapping

* `Right Stick X` → Controls steering angle
* `Left Stick Y` → Controls speed (forward/backward)

### Sensor Layout

* **IR Line Sensors**: `left_far`, `left`, `center`, `right`, `right_far`
* **TCS3200 Color Sensor**: Outputs RGB values based on surface color
* **Servo Steering**: Mimics vehicle-like steering angles
* **PWM Motor Drive**: Uses two channels for controlling left and right wheels

---

## Pin Configuration

### Motors

| Component           | ESP32 Pin |
| ------------------- | --------- |
| Left Motor Logic 1  | 27        |
| Left Motor Logic 2  | 26        |
| Right Motor Logic 1 | 17        |
| Right Motor Logic 2 | 5         |
| Left Motor PWM      | 25        |
| Right Motor PWM     | 16        |

### Servo

| Component      | ESP32 Pin |
| -------------- | --------- |
| Steering Servo | 22        |

### IR Sensors

| Sensor    | ESP32 Pin |
| --------- | --------- |
| Left Far  | 35        |
| Left      | 32        |
| Center    | 34        |
| Right     | 39        |
| Right Far | 36        |

### TCS3200 Color Sensor

| Pin | ESP32 Pin |
| --- | --------- |
| S2  | 0         |
| S3  | 4         |
| OUT | 2         |

---

## Network Configuration

Set in `setup()`:

```cpp
IPAddress local_IP(192,168,80,32);
IPAddress gateway(192,168,80,23);
IPAddress subnet(255,255,255,0);
const char* ssid = "KOTB_MH";
const char* password = "12345678";
```

* **LabVIEW Client (RGB Output)**: TCP Server on port `32`
* **Command Output (Debug, IR, etc.)**: TCP Server on port `33`

---

## Dependencies

* [ESP32 Servo Library](https://github.com/madhephaestus/ESP32Servo)
* [PS3Controller Library](https://github.com/jvpernis/esp32-ps3)
* WiFi and WiFiServer (included in ESP32 core)

---

## Setup & Usage

1. Connect the ESP32 to the PC and upload the code using Arduino IDE.
2. Connect the PS3 controller (set MAC address in `Ps3.begin()`).
3. Join the ESP32 to the correct Wi-Fi network.
4. Run your LabVIEW or TCP client and connect to:

   * **Port 32**: To receive RGB readings.
   * **Port 33**: To monitor debug messages or IR sensor status.
5. Use the PS3 controller to switch modes and control the robot.

---

## Example Output

### RGB Output Format

```
01234,01567,01123
```

### IR Sensor State Format

```
1,0,1,0,1  // Corresponding to [left_far, left, center, right, right_far]
```

---

## Notes

* Servo angle mapping is dynamically adjusted based on joystick X-axis.
* Steering calculations simulate turning radii using trigonometry (`tan(θ)`).
* Wi-Fi server design allows parallel monitoring and command communication.

---

## License

This project is open-source under the MIT License.
