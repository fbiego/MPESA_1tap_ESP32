# MPESA_1tap_ESP32

M-PESA 1Tap on ESP32

[`Demo Video`](https://www.linkedin.com/posts/fbiego_esp32-rfid-mpesa-activity-6808298597168254976-g6PV)

This demo uses
- RFID reader & tag
- OLED screen
- Button (for random amount generator)

### Pinout 
| ESP32 | MFRC522| | ESP32 | OLED |
| --- | --- | --- | --- | --- |
| 18 | SCK | | 4 | SCL |
| 19 | MISO | | 5 | SDA |
| 21 | SDA | | 15 | BUTTON |
| 22 | RST | | 2 | BUZZER |
| 23 | MOSI | | | |

