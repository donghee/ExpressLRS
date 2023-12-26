#!/bin/sh

# gdbgui --gdb-cmd="gdb-multiarch -n -x .pioinit-rx-gcm" --args ./.pio/build/LEA_2400_RX_GCM_TEST/firmware.bin
# gdbfrontend -g $(realpath /usr/bin/gdb-multiarch) --gdb-args="-n -x .pioinit-rx-gcm"

case "$1" in
  "TX")
    gdb-multiarch -x .pioinit-tx-gcm
    ;;
	"RX")
    gdb-multiarch -x .pioinit-rx-gcm
    ;;
	 *)
     echo "TX or RX"
     exit 1
     ;;
esac
