# chromebook-enroll

[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](#license)
[![Last commit](https://img.shields.io/github/last-commit/davidribeiro-dev/chromebook-enroll)](https://github.com/davidribeiro-dev/chromebook-enroll/commits/main)
[![Made with Arduino](https://img.shields.io/badge/Made%20with-Arduino-00979D?logo=arduino)](https://www.arduino.cc/)
[![Forked from Centipede](https://img.shields.io/badge/forked%20from-Centipede-orange)](http://labs.amplifiedit.com/centipede)

> An Arduino HID tool that automated Chromebook enrollment for Blue Hills Regional Technical — used to provision **over 300 devices in two days**, cutting per-device enrollment time by **~70%**.

## Contents

- [Overview](#overview)
- [Results](#results)
- [How It Works](#how-it-works)
- [What's Different from Upstream Centipede](#whats-different-from-upstream-centipede)
- [Hardware](#hardware)
- [Setup](#setup)
- [Usage](#usage)
- [Reliability](#reliability)
- [What I Learned](#what-i-learned)
- [Acknowledgements](#acknowledgements)
- [License](#license)

## Overview

Chromebook enrollment is slow and repetitive: each device takes an IT staff member 5–10 minutes of walking through the same OOBE screens — Wi-Fi setup, the `Ctrl+Alt+E` enrollment shortcut, credential entry, waiting for the configuration download. For one device it's fine; for a school district provisioning hundreds, it becomes hours of manual keyboard work.

This project, deployed at Blue Hills Regional Technical, automated the flow using an Arduino Pro Micro as a USB HID keyboard. Plug the Arduino into a freshly-powerwashed Chromebook, and the board takes over — connects to Wi-Fi, triggers enrollment, types credentials, waits for configuration to download, and signals completion via the on-board LED.

## Results

| Metric | Value |
| --- | --- |
| Devices enrolled | **300+** |
| Time window | **~2 days** |
| Per-device time, manual | 5–10 minutes |
| Per-device time, automated | ~2–3 minutes |
| Time reduction | **~70%** |
| Failure rate | ~10 / 300 (~3%) |
| Estimated IT staff time saved | **~25 hours** |

## How It Works

1. The Arduino enumerates as a USB HID keyboard.
2. On boot it waits a few seconds for the ChromeOS OOBE to fully load.
3. It walks the **welcome → Wi-Fi → enrollment → credentials** flow by sending the same key sequences (Tab, Down-Arrow, Enter, character input) a human operator would press.
4. After triggering enrollment with `Ctrl+Alt+E` and typing credentials, it waits ~60 seconds for configuration to finish downloading, then signals success via the on-board LED.

A debug pin (D2 → GND) skips the flow for safe testing.

## What's Different from Upstream Centipede

This sketch is a **trimmed fork of [Centipede](http://labs.amplifiedit.com/centipede)** (CDW Amplified for Education, MIT License) — the de facto open-source Chromebook enrollment automator.

The version handed to me did not work for our ChromeOS 137 fleet — the UI navigation sequences (tab counts, down-arrow counts, timing windows) were tuned for older OOBE flows. I rebuilt them by:

1. **Reading through the upstream Centipede code** to understand the architecture and intent of each function.
2. **Stepping through the ChromeOS 137 OOBE manually**, counting the keystrokes required between each screen and timing how long each transition took.
3. **Patching the broken sequences and re-testing** on a powerwashed device, iterating until the flow ran reliably.

Once it was stable in production, I trimmed the codebase to just what we needed:

| Removed | Reason |
| --- | --- |
| Version-compat branches (pre-137) | We deploy only ChromeOS 137 |
| Advanced EAP (PEAP, LEAP, EAP-TLS, EAP-TTLS) | Our SSID is WPA2/PSK |
| Certificate enrollment | Not part of our flow |
| Powerwash function | We powerwash before plugging in |
| Retry logic | Re-plug recovers cleanly |
| Sign-in, remove-enrollment-wifi | Out of scope for first-boot enrollment |
| ToS walker, ChromeVox escape, showVersion | Not needed for our flow |

Result: same proven flow, ~70% smaller codebase, easier to read and maintain.

## Hardware

| Component | Notes |
| --- | --- |
| Microcontroller | Arduino Pro Micro (or Leonardo / any ATmega32U4 board with native HID support) |
| Cable | USB-A to micro-USB (data, not charge-only) |
| Optional | Jumper wire for the debug pin (D2 → GND) |

A regular Arduino Uno or Nano **will not work** — they lack native HID and cannot emulate a keyboard without additional firmware.

## Setup

1. Clone the repository:
```bash
   git clone https://github.com/davidribeiro-dev/chromebook-enroll.git
```
2. Copy `config.h.example` to `config.h` and fill in your real values:
```cpp
   #define WIFI_SSID      "YourSchoolWiFi"
   #define WIFI_PASSWORD  "YourWiFiPassword"
   #define USER_EMAIL     "you@school.org"
   #define USER_PASSWORD  "YourEnrollmentPassword"
```
   `config.h` is gitignored — credentials never reach the repo.
3. Open `chromebook-enroll.ino` in the Arduino IDE.
4. Select your board (**Pro Micro** / **Leonardo**) and the matching port.
5. Upload.

## Usage

1. Powerwash the target Chromebook and wait for the OOBE welcome screen.
2. Plug the flashed Arduino into the Chromebook.
3. The board takes over. The on-board LED blinks during the flow and turns solid on success.
4. ~2–3 minutes later the device is enrolled and ready.

For safe testing without running the flow, jumper pin **D2 → GND** before plugging in.

## Reliability

Across the production run (300+ devices over two days):

- **~97% completed with no intervention.**
- **~3% (~10 devices)** needed manual recovery, generally caused by an unexpected post-update screen or a brief disconnection during the flow.
- **Recovery procedure:** power-cycle back to the first OOBE screen, re-plug the Arduino, re-run. All affected devices completed on a second attempt.

## What I Learned

- **HID timing is fragile.** Different Chromebooks complete the same OOBE step in different amounts of time. The right pause length is whichever one works for the slowest device in your fleet.
- **Read before rewriting.** Centipede was years of accumulated tribal knowledge about ChromeOS OOBE quirks. Reading through it carefully was faster than starting from a blank file — even though most of what I read I ended up cutting.
- **Secrets management starts on day one.** Hardcoded credentials in source are how district passwords end up on the public internet. Externalizing them to a gitignored `config.h` was the first thing I did when forking.

## Acknowledgements

- **[Centipede](http://labs.amplifiedit.com/centipede)** by CDW Amplified for Education (MIT License) — the upstream project this fork is derived from.

## License

This project is dual-credited:

- Original Centipede code © 2023 CDW Amplified for Education
- Trimmed fork, debugging, and additions © 2026 David Ribeiro

Both released under the **MIT License** — see [LICENSE](LICENSE) for details.
