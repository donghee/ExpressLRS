#!/bin/sh

case "$1" in
  "TX")
    pio run -e LEA_2400_TX_GCM_TEST --target clean && pio debug -e LEA_2400_TX_GCM_TEST
    ;;
	"RX")
    pio run -e LEA_2400_RX_GCM_TEST --target clean && pio debug -e LEA_2400_RX_GCM_TEST
    ;;
	 *)
     echo "TX or RX"
     exit 1
     ;;
esac
