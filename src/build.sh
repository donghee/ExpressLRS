#!/bin/sh

./mbedtls_patch.sh

case "$1" in
  "TX")
pio run -e LEA_2400_TX_via_STLINK --target clean && pio debug -e LEA_2400_TX_via_STLINK
    ;;
	"RX")
pio run -e LEA_2400_RX_via_STLINK --target clean && pio debug -e LEA_2400_RX_via_STLINK
    ;;
	 *)
     echo "TX or RX"
     exit 1
     ;;
esac

