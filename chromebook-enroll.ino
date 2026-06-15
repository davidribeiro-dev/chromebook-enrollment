/*
  chromebook-enroll.ino

  Trimmed Chromebook enrollment automator for Blue Hills Regional Technical.

  Forked from Centipede (CDW Amplified for Education, MIT License):
    http://labs.amplifiedit.com/centipede

  Stripped to one ChromeOS version (137) and one Wi-Fi auth type (WPA2).
  Removed: version-compat branches (pre-137), advanced EAP, certificate
  enrollment, Powerwash, retry, sign-in, ToS walker, remove-enrollment-wifi.

  Hardware: Arduino Pro Micro / Leonardo (ATmega32U4 with HID).
  Drop a wire between pin 2 and GND to skip the flow (debug mode).

  © 2023 CDW Amplified for Education (upstream)
  © 2026 David Ribeiro (trimmed fork)
  MIT License — https://opensource.org/licenses/MIT
*/

#include <Keyboard.h>
#include "config.h"

// Special key codes (carried over from Centipede)
#define KEY_LEFT_CTRL   0x80
#define KEY_LEFT_ALT    0x82
#define KEY_DOWN_ARROW  0xD9
#define KEY_TAB         0xB3
#define KEY_ENTER       0xB0

const int buttonPin = 2;   // Ground to skip flow
const int RXLED     = 17;
const int slowMode  = 1;   // Set 0 once timing is dialed in

void setup() {
  setPrescaler();
  Keyboard.begin();

  pinMode(buttonPin, INPUT);
  digitalWrite(buttonPin, HIGH);
  pinMode(RXLED, OUTPUT);
  digitalWrite(RXLED, LOW);
  TXLED1;

  wait(5);
}

void loop() {
  if (digitalRead(buttonPin) == 1) {
    enterEnrollment();
    wifiConfig();
    wait(5);

    TXLED1;
    wait(25);                // Device configuration download
    TXLED0;

    enterCredentials();
    wait(9);
    Keyboard.write(KEY_ENTER);
    wait(60);                // Enrollment completion

    showSuccess();
  }
  bootLoop();
}

// ── Enrollment shortcut: Ctrl+Alt+E ──────────────────────────────
void enterEnrollment() {
  Keyboard.press(KEY_LEFT_CTRL);
  Keyboard.press(KEY_LEFT_ALT);
  Keyboard.write('e');
  Keyboard.release(KEY_LEFT_ALT);
  Keyboard.release(KEY_LEFT_CTRL);
  wait(2);
}

// ── Wi-Fi setup: WPA2 only ───────────────────────────────────────
void wifiConfig() {
  repeatKey(KEY_TAB, 7);
  wait(2);
  Keyboard.write(KEY_ENTER);
  wait(2);

  // Walk down to "Add Wi-Fi"
  repeatKey(KEY_DOWN_ARROW, 20);
  Keyboard.write(KEY_ENTER);
  wait(2);

  // SSID
  Keyboard.print(WIFI_SSID);
  Keyboard.write(KEY_TAB);
  wait(2);

  // Security: WPA2 (2 down arrows from default)
  repeatKey(KEY_DOWN_ARROW, 2);
  Keyboard.write(KEY_TAB);
  wait(2);

  // Password
  Keyboard.print(WIFI_PASSWORD);
  repeatKey(KEY_TAB, 3);
  wait(2);

  Keyboard.write(KEY_ENTER);  // Connect
  wait(6);
  Keyboard.write(KEY_TAB);
  Keyboard.write(KEY_ENTER);
  wait(8);
}

// ── Type enrollment credentials ──────────────────────────────────
void enterCredentials() {
  wait(5);
  Keyboard.print(USER_EMAIL);
  wait(3);
  Keyboard.write(KEY_ENTER);
  wait(8);

  Keyboard.print(USER_EMAIL);
  Keyboard.write(KEY_ENTER);
  wait(3);

  Keyboard.print(USER_PASSWORD);
  wait(3);
  Keyboard.write(KEY_ENTER);
  wait(3);
}

// ── Status / utility helpers ─────────────────────────────────────
void bootLoop() {
  TXLED0; delay(200);
  TXLED1; delay(200);
  TXLED0; delay(200);
  TXLED1; delay(800);
}

void showSuccess() {
  digitalWrite(RXLED, HIGH);
  while (true) bootLoop();
}

void repeatKey(byte key, int num) {
  for (int i = 0; i < num; i++) {
    Keyboard.write(key);
    wait(1);
  }
}

void blink() {
  digitalWrite(RXLED, LOW);  delay(250);
  digitalWrite(RXLED, HIGH); delay(250);
}

void wait(int cycles) {
  for (int i = 0; i < cycles; i++) {
    blink();
    if (slowMode) delay(250);
  }
}

void setPrescaler() {
  uint8_t oldSREG = SREG;
  cli();
  CLKPR = _BV(CLKPCE);
  CLKPR = 0x0;
  SREG = oldSREG;
}
