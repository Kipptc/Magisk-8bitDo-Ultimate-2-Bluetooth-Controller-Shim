#!/system/bin/sh
MODDIR=${1:-/data/adb/modules/padshimboot}
TARGET_NAME="Nintendo Switch Pro Controller"

controller_present() {
    grep -Fq "$TARGET_NAME" /proc/bus/input/devices 2>/dev/null
}

start_padshim() {
    chmod 0755 "$MODDIR/bin/padshim" 2>/dev/null
    "$MODDIR/bin/padshim" >/dev/null 2>&1 &
    PADSHIM_PID=$!
}

until [ "$(getprop sys.boot_completed)" = "1" ]; do
    sleep 2
done

until [ -e /dev/uinput ]; do
    sleep 2
done

pkill -f "$MODDIR/bin/padshim" 2>/dev/null

while true; do
    while ! controller_present; do
        sleep 1
    done

    start_padshim

    while kill -0 "$PADSHIM_PID" 2>/dev/null; do
        sleep 1
    done

    while controller_present; do
        sleep 1
    done
done
