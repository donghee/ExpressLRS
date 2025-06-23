#!/bin/sh

./mbedtls_patch.sh

case "$1" in
  "TX")
$HOME/.platformio/penv/bin/pio run -e LEA_2400_TX_via_STLINK --target clean && $HOME/.platformio/penv/bin/pio debug -e LEA_2400_TX_via_STLINK
    ;;
	"RX")
$HOME/.platformio/penv/bin/pio run -e LEA_2400_RX_via_STLINK --target clean && $HOME/.platformio/penv/bin/pio debug -e LEA_2400_RX_via_STLINK
    ;;
  "AIO_TX")
$HOME/.platformio/penv/bin/pio run -e LEA_2400_AIO_TX_via_STLINK --target clean && $HOME/.platformio/penv/bin/pio debug -e LEA_2400_AIO_TX_via_STLINK
    ;;
	"AIO_RX")
$HOME/.platformio/penv/bin/pio run -e LEA_2400_AIO_RX_via_STLINK --target clean && $HOME/.platformio/penv/bin/pio debug -e LEA_2400_AIO_RX_via_STLINK
    ;;

	 *)
     echo "TX or RX or AIO_TX or AIO_RX must be specified as an argument."
     exit 1
     ;;
esac

