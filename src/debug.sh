#!/bin/sh

case "$1" in
  "TX")
    gdb-multiarch -x .pioinit-tx
    ;;
	"RX")
    gdb-multiarch -x .pioinit-rx
    ;;
	 *)
     echo "TX or RX"
     exit 1
     ;;
esac
