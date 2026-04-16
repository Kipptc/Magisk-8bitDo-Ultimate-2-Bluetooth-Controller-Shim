# Controller Shim - Padshim

Padshim is a lightweight root utility for Android that creates a virtual gamepad using Linux `uinput` and fixes controller compatibility issues caused by non-standard trigger reporting.

The bluetooth functionality of this controller is really meant to emulate a Switch Pro controller for use on the Switch.
This doesn't work for Android, so you just simply can't use the triggers in a lot of apps - my usecase was Gamehub.

Padshim intercepts the original controller input and exposes a corrected virtual controller with working triggers.

It also modifies the face buttons to actually match their necessary inputs - such as:
* South face is read as A
* East face is read as B
* North face is read as Y
* West face is read as X

The triggers are solved by forcing both BUTTON_TL2 and BUTTON_TR2 to report as BUTTON_L2/R2 as well as ABS_BRAKE and ABS_GAS.
Sadly I can't force the controller to respect true ABS_GAS/ABS_BRAKE pressure values while in bluetooth mode, so it forces a full 1.0 value.

This may even work for wired if you want, but I didn't test that.

---

# Features

- Auto-detects controller by device name:
  - `Nintendo Switch Pro Controller`
- Automatically handles changing `/dev/input/eventX` input reporting to applications
- Sleep cycle of 30 seconds between checks so the script isn't just firing off forever in the background
- Grabs original device events so no doubled inputs
- Creates virtual controller via `uinput`
- Converts digital triggers into analog trigger axes:
  - L2 → `ABS_BRAKE`
  - R2 → `ABS_GAS`
- Also forwards original L2 / R2 button presses
- Face buttons remapped for proper Android layout

---

# Requirements

- Root access
- Android device with `/dev/uinput`
- Magisk recommended

---

# Build

Just compile directly on-device with Termux:

```sh
pkg install clang
clang -O2 -o padshim padshim.c
