# PICKLE RICK Firmware - Execution Plan

## Rick & Morty Themed WiFi Security Tool for K257 T-LoRa Pager

**Inspired by:** M5PORKCHOP by 0ct0sec
**Target Hardware:** LILYGO T-LoRa Pager (K257) - ESP32-S3
**Theme:** Rick Sanchez / Space / Interdimensional

---

## Hardware Compatibility Analysis

### M5Cardputer (PORKCHOP) vs K257 (PICKLE RICK)

| Feature | M5Cardputer | K257 T-LoRa Pager | Compatible |
|---------|-------------|-------------------|------------|
| MCU | ESP32-S3 | ESP32-S3 | **YES** |
| Flash | 8MB | 16MB | **YES** (more!) |
| PSRAM | None | 8MB QSPI | **YES** (better!) |
| Display | 240x135 ST7789 | 480x222 ST7796 | Adapt UI |
| WiFi | 802.11 b/g/n | 802.11 b/g/n | **YES** |
| BLE | BLE 5.0 | BLE 5.0 | **YES** |
| GPS | External (optional) | Built-in | **YES** (better!) |
| LoRa | None | SX1262 915MHz | **BONUS!** |
| Keyboard | Mini QWERTY | Full QWERTY | **YES** (better!) |
| SD Card | Yes | Yes | **YES** |
| Audio | Buzzer | ES8311 Codec | **YES** (better!) |
| Haptic | None | DRV2605 | **YES** (bonus!) |

### K257 Exclusive Features
- **LoRa SX1262**: Mesh networking between devices (Council of Ricks mode)
- **8MB PSRAM**: More buffers for packet capture
- **Full Keyboard**: Better navigation and input
- **Haptic Feedback**: Tactile confirmations
- **ES8311 Audio**: Sound effects for achievements

---

## Feature Rebranding: PORKCHOP -> PICKLE RICK

| PORKCHOP Mode | PICKLE RICK Mode | Description |
|---------------|------------------|-------------|
| OINK Mode | **PORTAL GUN** | WiFi passive scanning |
| Handshake Capture | **INTERDIMENSIONAL CABLE** | WPA/WPA2 capture |
| PIGGY BLUES | **GET SCHWIFTY** | BLE spam attacks |
| SGT WARHOG | **WUBBA LUBBA DUB DUB** | GPS wardriving |
| CHILL DONOHAM | **CHILL WITH UNITY** | Passive mode |
| Spectrum | **MICROVERSE** | 2.4GHz spectrum view |
| PORKCHOP COMMANDER | **PLUMBUS COMMANDER** | File manager |
| N/A (new) | **COUNCIL OF RICKS** | LoRa mesh (K257 exclusive!) |

| PORKCHOP Concept | PICKLE RICK Concept |
|------------------|---------------------|
| Piglet Avatar | **Rick Avatar** |
| Pig Moods | **Rick Moods** (Drunk, Genius, Angry, etc.) |
| Pig Ranks | **Rick & Morty Ranks** (Morty -> Multiverse God) |
| Pig Achievements | **R&M Achievements** (Wubba Lubba, Get Schwifty, etc.) |

---

## Phase 1: Foundation (Core Framework)

### 1.1 Project Setup
- [x] Create project directory structure
- [x] Copy K257 board definitions from pager-ai
- [x] Configure platformio.ini for K257
- [x] Create config.h with all defines
- [x] Set up partition scheme (16MB flash)

### 1.2 Hardware Abstraction
- [ ] Initialize LilyGoLib for K257
- [ ] Configure display (ST7796 480x222)
- [ ] Set up keyboard (TCA8418)
- [ ] Configure rotary encoder
- [ ] Initialize haptic feedback (DRV2605)
- [ ] Set up SD card (SPI)
- [ ] Initialize GPS (UART)
- [ ] Initialize LoRa (SX1262)
- [ ] Configure audio codec (ES8311)

### 1.3 Core Systems
- [ ] Implement config loading/saving
- [ ] Create SD logging system
- [ ] Implement XP/rank persistence
- [ ] Create state machine

---

## Phase 2: WiFi Security Features

### 2.1 WiFi Scanner (Portal Gun Mode)
```
Files: src/wifi/wifi_scanner.cpp/.h
```
- [ ] Implement passive WiFi scanning
- [ ] Channel hopping (1-13)
- [ ] Network list management
- [ ] Auth mode detection
- [ ] Hidden SSID detection
- [ ] Signal strength tracking
- [ ] MAC randomization
- [ ] WiGLE CSV export

### 2.2 Handshake Capture (Interdimensional Cable Mode)
```
Files: src/wifi/handshake_capture.cpp/.h
```
- [ ] Enable promiscuous mode
- [ ] EAPOL frame detection
- [ ] 4-way handshake reconstruction
- [ ] PMKID extraction from beacons
- [ ] Hashcat 22000 format export
- [ ] hccapx format export
- [ ] Target selection
- [ ] Capture notifications

### 2.3 Deauthentication (Optional)
```
Files: src/wifi/deauth.cpp/.h
```
- [ ] Deauth frame construction
- [ ] Single client deauth
- [ ] Broadcast deauth
- [ ] Reason code selection
- [ ] Frame count configuration
- [ ] Safety limits

---

## Phase 3: BLE Features

### 3.1 BLE Spam (Get Schwifty Mode)
```
Files: src/ble/ble_spam.cpp/.h
```
- [ ] Initialize NimBLE
- [ ] Apple AirPods popup spam
- [ ] Apple AirDrop spam
- [ ] Google Fast Pair spam
- [ ] Samsung SmartTag spam
- [ ] Windows Swift Pair spam
- [ ] Random payload generation
- [ ] Configurable intervals
- [ ] TX power control

---

## Phase 4: GPS & Wardriving

### 4.1 GPS Module
```
Files: src/gps/gps_manager.cpp/.h
```
- [ ] UART communication with GPS
- [ ] NMEA parsing (TinyGPSPlus)
- [ ] Fix status monitoring
- [ ] Accuracy estimation
- [ ] Coordinate formatting

### 4.2 Wardriving (Wubba Lubba Dub Dub Mode)
```
Files: src/gps/wardriving.cpp/.h
```
- [ ] GPS + WiFi scan integration
- [ ] Distance tracking (Haversine)
- [ ] Session management
- [ ] Real-time map display
- [ ] WiGLE CSV export
- [ ] KML export (Google Earth)
- [ ] Statistics tracking

---

## Phase 5: LoRa Features (K257 Exclusive!)

### 5.1 LoRa Module
```
Files: src/lora/lora_manager.cpp/.h
```
- [ ] Initialize SX1262 (RadioLib)
- [ ] Configure frequency/bandwidth
- [ ] TX/RX operations
- [ ] Power control

### 5.2 Council of Ricks (LoRa Mesh Mode)
```
Files: src/lora/council_mesh.cpp/.h
```
- [ ] Device discovery
- [ ] Mesh beacon protocol
- [ ] Handshake sharing
- [ ] Network sharing
- [ ] XP sync between devices
- [ ] Achievement unlocks via mesh
- [ ] Encrypted communication

---

## Phase 6: Gamification System

### 6.1 Rick Avatar
```
Files: src/core/pickle_rick.cpp/.h
```
- [ ] Avatar state machine
- [ ] Expression system (10 expressions)
- [ ] Blink animations
- [ ] Mood transitions
- [ ] Quote system
- [ ] Activity reactions

### 6.2 Mood System
```
Files: src/core/mood.cpp/.h
```
- [ ] Mood calculation
- [ ] XP multipliers
- [ ] Scan speed modifiers
- [ ] Random mood events
- [ ] Mood persistence

### 6.3 XP & Rank System
```
Files: src/core/xp_system.cpp/.h
```
- [ ] XP award with modifiers
- [ ] Rank thresholds (40 levels)
- [ ] Rank up animations
- [ ] XP persistence to SD

### 6.4 Achievements
```
Files: src/core/achievements.cpp/.h
```
- [ ] 63 Rick & Morty themed achievements
- [ ] Progress tracking
- [ ] Unlock notifications
- [ ] SD persistence

### 6.5 Challenges
```
Files: src/core/challenges.cpp/.h
```
- [ ] Daily challenge generation
- [ ] Progress tracking
- [ ] Completion rewards
- [ ] Refresh timer

---

## Phase 7: UI (LVGL Space Theme)

### 7.1 Theme Setup
```
Files: src/ui/theme.cpp/.h
```
- [ ] Portal green color palette
- [ ] Space black background
- [ ] Star animations
- [ ] Portal swirl effects
- [ ] Custom fonts

### 7.2 Boot Screen
- [ ] ASCII art Rick face
- [ ] Version info
- [ ] Loading progress
- [ ] Random boot quote

### 7.3 Main Menu (Rick's Garage)
- [ ] Mode selection grid
- [ ] Rick avatar display
- [ ] XP/Rank bar
- [ ] Status icons
- [ ] Animated background

### 7.4 Mode Screens
- [ ] Portal Gun screen (network list)
- [ ] Interdimensional screen (capture status)
- [ ] Get Schwifty screen (spam controls)
- [ ] Wubba Lubba screen (GPS map)
- [ ] Microverse screen (spectrum graph)
- [ ] Council of Ricks screen (mesh status)
- [ ] Plumbus screen (file browser)

### 7.5 Overlays
- [ ] Achievement popup
- [ ] Rank up animation
- [ ] Notification toast
- [ ] Rick quote bubble

---

## Phase 8: Audio & Haptics

### 8.1 Sound Effects
```
Files: src/audio/sfx.cpp/.h
```
- [ ] Portal open sound
- [ ] Handshake capture sound
- [ ] Level up fanfare
- [ ] Achievement unlock
- [ ] "Wubba lubba dub dub" audio clip
- [ ] Button click sounds

### 8.2 Haptic Feedback
```
Files: src/haptics/haptics.cpp/.h
```
- [ ] Button press feedback
- [ ] Capture success vibration
- [ ] Achievement unlock pattern
- [ ] Error feedback

---

## Phase 9: File Management

### 9.1 Plumbus Commander
```
Files: src/modes/plumbus.cpp/.h
```
- [ ] Dual-pane file browser
- [ ] File operations (copy, move, delete)
- [ ] Handshake file viewer
- [ ] Export to USB mass storage
- [ ] SD card stats

---

## Phase 10: Polish & Release

### 10.1 Testing
- [ ] All modes functional
- [ ] No memory leaks
- [ ] Stable long-term operation
- [ ] Battery life optimization

### 10.2 Documentation
- [ ] README with features
- [ ] User manual
- [ ] Build instructions
- [ ] Asset creation guide

### 10.3 Release
- [ ] GitHub repo setup
- [ ] Pre-built firmware binary
- [ ] OTA update support

---

## Asset Requirements

### Graphics (C arrays for LVGL)
1. **Rick Avatar Sprites**
   - 10 expressions at 64x64 pixels
   - Portal effect overlay

2. **Mode Icons**
   - Portal Gun (green spiral)
   - Interdimensional TV
   - Music notes (Schwifty)
   - GPS marker (Wubba Lubba)
   - Spectrum wave
   - Multiple Ricks (Council)
   - Plumbus

3. **Rank Icons**
   - 21 unique icons for each rank

4. **Background**
   - Animated starfield
   - Portal swirl animation

### Audio Files (WAV, SD card)
1. Portal opening sound
2. Handshake capture beep
3. Level up fanfare
4. Achievement unlock
5. Rick quotes (optional)

---

## Implementation Priority

1. **HIGH**: WiFi Scanner + Handshake Capture (core functionality)
2. **HIGH**: LVGL UI with basic screens
3. **MEDIUM**: BLE Spam
4. **MEDIUM**: GPS Wardriving
5. **MEDIUM**: Gamification (XP, Ranks)
6. **LOW**: LoRa Mesh
7. **LOW**: Audio effects
8. **LOW**: Full achievement system

---

## Build & Flash Instructions

```bash
# Clone repo
git clone https://github.com/Averroeskw/pickle-rick-firmware.git
cd pickle-rick-firmware

# Build
pio run -e pickle_rick

# Flash
pio run -e pickle_rick -t upload

# Monitor
pio device monitor
```

---

## Credits

- **Original PORKCHOP**: 0ct0sec (https://github.com/0ct0sec/M5PORKCHOP)
- **K257 Hardware**: LilyGo / Xinyuan
- **Base Firmware**: AVERROES Tech pager-ai
- **Rick & Morty**: Adult Swim / Dan Harmon & Justin Roiland

---

*"Nobody exists on purpose. Nobody belongs anywhere. Everybody's gonna die. Come wardrive with me."* - Rick Sanchez (probably)
