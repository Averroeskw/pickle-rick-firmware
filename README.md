# PICKLE RICK Firmware

<div align="center">
  <pre>
  ██████╗ ██╗ ██████╗██╗  ██╗██╗     ███████╗    ██████╗ ██╗ ██████╗██╗  ██╗
  ██╔══██╗██║██╔════╝██║ ██╔╝██║     ██╔════╝    ██╔══██╗██║██╔════╝██║ ██╔╝
  ██████╔╝██║██║     █████╔╝ ██║     █████╗      ██████╔╝██║██║     █████╔╝
  ██╔═══╝ ██║██║     ██╔═██╗ ██║     ██╔══╝      ██╔══██╗██║██║     ██╔═██╗
  ██║     ██║╚██████╗██║  ██╗███████╗███████╗    ██║  ██║██║╚██████╗██║  ██╗
  ╚═╝     ╚═╝ ╚═════╝╚═╝  ╚═╝╚══════╝╚══════╝    ╚═╝  ╚═╝╚═╝ ╚═════╝╚═╝  ╚═╝
  </pre>

  **WiFi Security Tool for LILYGO T-LoRa Pager (K257)**

  *"I turned myself into a WiFi security tool, Morty!"*

  ![Version](https://img.shields.io/badge/version-1.0.0-green)
  ![Platform](https://img.shields.io/badge/platform-ESP32--S3-blue)
  ![License](https://img.shields.io/badge/license-MIT-orange)
</div>

---

## Overview

PICKLE RICK is a Rick & Morty themed WiFi security research tool for the LILYGO T-LoRa Pager (K257). Inspired by [M5PORKCHOP](https://github.com/0ct0sec/M5PORKCHOP), this firmware brings the same powerful features to the K257 platform with Rick Sanchez as your guide through the wireless multiverse.

**Note:** This tool is for educational and authorized security research only. Always get permission before testing on networks you don't own.

---

## Features

### Operational Modes

| Mode | Name | Description |
|------|------|-------------|
| 🌀 | **PORTAL GUN** | Passive WiFi scanning with channel hopping |
| 📺 | **INTERDIMENSIONAL CABLE** | WPA/WPA2 handshake capture & PMKID extraction |
| 🎵 | **GET SCHWIFTY** | BLE advertisement spam (Apple, Android, Samsung, Windows) |
| 🚗 | **WUBBA LUBBA DUB DUB** | GPS-enabled wardriving with WiGLE export |
| 😌 | **CHILL WITH UNITY** | Passive observation mode |
| 🔬 | **MICROVERSE** | 2.4GHz spectrum visualization |
| 👥 | **COUNCIL OF RICKS** | LoRa mesh networking (K257 exclusive!) |
| 🔧 | **PLUMBUS COMMANDER** | SD card file management |

### Gamification

- **40 Ranks**: Progress from Morty to Multiverse God
- **63 Achievements**: Unlock Rick & Morty themed achievements
- **Daily Challenges**: Complete missions for bonus XP
- **Rick's Mood System**: Mood affects XP multipliers and scan speed
- **Persistent XP**: Progress saved to SD card

### K257 Exclusive Features

- **LoRa SX1262**: Share captures between devices via mesh network
- **8MB PSRAM**: Large packet buffers for reliable capture
- **Full Keyboard**: Easy navigation and input
- **Haptic Feedback**: DRV2605 for tactile confirmations
- **ES8311 Audio**: Sound effects for achievements

---

## Hardware Requirements

- **LILYGO T-LoRa Pager (K257)**
  - ESP32-S3 with 16MB Flash, 8MB PSRAM
  - ST7796 480x222 Display
  - SX1262 LoRa Module
  - GPS Module
  - Full QWERTY Keyboard
  - SD Card Slot

---

## Installation

### Prerequisites

- [PlatformIO](https://platformio.org/) (VSCode extension or CLI)
- USB-C cable

### Build & Flash

```bash
# Clone the repository
git clone https://github.com/Averroeskw/pickle-rick-firmware.git
cd pickle-rick-firmware

# Build firmware
pio run -e pickle_rick

# Flash to device
pio run -e pickle_rick -t upload

# Monitor serial output
pio device monitor
```

### Enter Download Mode (if needed)

If the K257 USB port keeps disconnecting:
1. Hold the BOOT button
2. Press and release RST button
3. Release BOOT button
4. Flash firmware

---

## Usage

### Navigation

- **Rotary Encoder**: Scroll through menus
- **Rotary Press**: Select item
- **ESC**: Return to previous screen
- **SPACE**: Mode-specific action
- **ENTER**: Confirm

### Quick Start

1. Power on the K257
2. Wait for boot sequence (Rick will greet you)
3. Use rotary to select a mode
4. Press rotary to enter mode
5. Capture handshakes, spam BLE, or wardrive!
6. Export data via SD card

---

## File Structure

```
pickle-rick-firmware/
├── src/
│   ├── main.cpp           # Entry point
│   ├── config.h           # Configuration
│   ├── core/              # Rick avatar, XP, achievements
│   ├── wifi/              # WiFi scanner, handshake capture
│   ├── ble/               # BLE spam
│   ├── gps/               # GPS & wardriving
│   ├── lora/              # LoRa mesh (Council of Ricks)
│   ├── ui/                # LVGL UI screens
│   └── modes/             # Mode implementations
├── boards/                # K257 board definition
├── variants/              # Pin definitions
├── assets/                # Graphics & sounds
├── docs/                  # Documentation
├── platformio.ini         # Build configuration
└── EXECUTION_PLAN.md      # Development roadmap
```

---

## Exported File Formats

| Data | Format | Compatible With |
|------|--------|-----------------|
| Handshakes | .22000 | Hashcat |
| Handshakes | .hccapx | Hashcat (legacy) |
| PMKID | .22000 | Hashcat |
| Wardriving | .csv | WiGLE |
| Wardriving | .kml | Google Earth |

---

## Ranks

| Level | Rank | XP Required |
|-------|------|-------------|
| 1-2 | Morty | 0 |
| 3-4 | Summer | 500 |
| 5-6 | Beth | 1,500 |
| 7-8 | Jerry | 3,000 |
| 9-10 | Mr. Meeseeks | 5,000 |
| 11-12 | Scary Terry | 8,000 |
| ... | ... | ... |
| 39-40 | Multiverse God | 330,000 |

---

## Credits

- **Original Inspiration**: [M5PORKCHOP](https://github.com/0ct0sec/M5PORKCHOP) by 0ct0sec
- **K257 Base**: [LilyGoLib](https://github.com/Xinyuan-LilyGO/LilyGoLib)
- **Firmware Development**: AVERROES Tech Manufacturing
- **Rick & Morty**: Adult Swim / Dan Harmon & Justin Roiland

---

## Disclaimer

This firmware is provided for **educational and authorized security research purposes only**. The developers are not responsible for any misuse. Always:

- Get explicit permission before testing any network
- Follow local laws regarding wireless security research
- Use this tool responsibly and ethically

*"Wubba lubba dub dub!"* translates to "I am in great pain, please help me" - which is how you'll feel if you use this tool illegally.

---

## License

MIT License - See [LICENSE](LICENSE) for details.

---

<div align="center">
  <i>"Nobody exists on purpose. Nobody belongs anywhere. Everybody's gonna die. Come wardrive with me."</i>
  <br><br>
  Made with 🥒 by AVERROES Tech
</div>
