#!/bin/sh

# gdbgui --gdb-cmd="gdb-multiarch -n -x .pioinit-rx" --args ./.pio/build/LEA_2400_RX_via_STLINK/firmware.bin
# gdbfrontend -g $(realpath /usr/bin/gdb-multiarch) --gdb-args="-n -x .pioinit-rx"

case "$1" in
  "TX")
    gdb-multiarch -x .pioinit-tx
    ;;
	"RX")
    gdb-multiarch -x .pioinit-rx
    ;;
  "AIO_TX")
    gdb-multiarch -x .pioinit-aio-tx
    ;;
	"AIO_RX")
    gdb-multiarch -x .pioinit-aio-rx
    ;;

	 *)
     echo "TX or RX"
     exit 1
     ;;
esac
