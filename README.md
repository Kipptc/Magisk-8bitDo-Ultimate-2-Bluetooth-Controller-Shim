# Controller Shim - Padshim

Padshim is a lightweight root utility for Android that creates a virtual gamepad using Linux `uinput` and fixes controller compatibility issues caused by non-standard trigger reporting.

The bluetooth functionality of this controller is really meant to emulate a Switch Pro controller for use on the Switch.
This doesn't work for Android, so you just simply can't use the triggers in a lot of apps.
My usecase was Gamehub - specifically the Bannerhub build: https://github.com/The412Banner/BannerHub

Padshim intercepts the original controller input and exposes a corrected virtual controller with working triggers.

The triggers are solved by forcing both BUTTON_TL2 and BUTTON_TR2 to report as BUTTON_L2/R2 as well as ABS_BRAKE and ABS_GAS.
Sadly I can't force the controller to respect true ABS_GAS/ABS_BRAKE pressure values while in bluetooth mode, so it forces a full 1.0 value.

Face buttons are also improperly configured for android, so that is intercepted and corrected as well.

---

# Features

- Auto-detects controller by device name:
  - `Nintendo Switch Pro Controller`
- Automatically handles changing `/dev/input/eventX` input reporting to applications
- Watches `/dev/input` for changes when possible so it can stay idle instead of firing forever in the background
- Falls back to timed waiting if the watch path isn't available on your Android build
- Grabs original device events so no doubled inputs
- Creates virtual controller via `uinput`
- Virtual controller shows up as `Cyanogenmon Padshim`
- Converts digital triggers into analog trigger axes:
  - L2 -> `ABS_BRAKE`
  - R2 -> `ABS_GAS`
  - Due to a limitation in how the controller's bluetooth mode, the trigger axes always report a full 1.0 value
- Also forwards original L2 / R2 button presses
- Face button inputs modified to match Android schema properly:
  - South face is read as A
  - East face is read as B
  - North face is read as Y
  - West face is read as X

---

# Install
Just load the zip through modules in magisk > reboot > done.

---

# Requirements

- Root access
- Android device with `/dev/uinput`
- Magisk recommended

---

# Build

Just compile directly on-device with Termux or in a Linux/WSL environment:

```sh
pkg install clang
clang -O2 -o padshim padshim.c
```

If you want debug logging while testing:

```sh
clang -O2 -DPADSHIM_DEBUG -o padshim padshim.c
```

---

# Future Plans

Hopefully throwing all of this into a UI for the user to edit their own on the fly.