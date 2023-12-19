#gdb-multiarch -x .pioinit-rx
#gdb-multiarch -x .pioinit-rx
#~/.platformio/packages/toolchain-gccarmnoneeabi/bin/arm-none-eabi-gdb -x .pioinit-rx

case "$1" in
  "TX")
    ~/.platformio/packages/toolchain-gccarmnoneeabi/bin/arm-none-eabi-gdb -x .pioinit-tx
    ;;
	"RX")
    ~/.platformio/packages/toolchain-gccarmnoneeabi/bin/arm-none-eabi-gdb -x .pioinit-rx
    ;;
	 *)
     echo "TX or RX"
     exit 1
     ;;
esac
