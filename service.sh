#!/system/bin/sh
MODDIR=${0%/*}
chmod 0755 "$MODDIR/watch-padshim.sh" 2>/dev/null
pkill -f "$MODDIR/watch-padshim.sh" 2>/dev/null
pkill -f "$MODDIR/bin/padshim" 2>/dev/null
sh "$MODDIR/watch-padshim.sh" "$MODDIR" >/dev/null 2>&1 &
