# gcm test
cp mbedtls_config.h .pio/libdeps/LEA_2400_TX_GCM_TEST/mbedtls/include/mbedtls/config.h
cp mbedtls_config.h .pio/libdeps/LEA_2400_RX_GCM_TEST/mbedtls/include/mbedtls/config.h

mv .pio/libdeps/LEA_2400_TX_GCM_TEST/mbedtls/library/common.h .pio/libdeps/LEA_2400_TX_GCM_TEST/mbedtls/library/mbedtls_common.h
mv .pio/libdeps/LEA_2400_RX_GCM_TEST/mbedtls/library/common.h .pio/libdeps/LEA_2400_RX_GCM_TEST/mbedtls/library/mbedtls_common.h

# main
cp mbedtls_config.h .pio/libdeps/LEA_2400_TX_via_STLINK/mbedtls/include/mbedtls/config.h
cp mbedtls_config.h .pio/libdeps/LEA_2400_RX_via_STLINK/mbedtls/include/mbedtls/config.h

mv .pio/libdeps/LEA_2400_TX_via_STLINK/mbedtls/library/common.h .pio/libdeps/LEA_2400_TX_via_STLINK/mbedtls/library/mbedtls_common.h
mv .pio/libdeps/LEA_2400_RX_via_STLINK/mbedtls/library/common.h .pio/libdeps/LEA_2400_RX_via_STLINK/mbedtls/library/mbedtls_common.h

# aio main
cp mbedtls_config.h .pio/libdeps/LEA_2400_AIO_TX_via_STLINK/mbedtls/include/mbedtls/config.h
cp mbedtls_config.h .pio/libdeps/LEA_2400_AIO_RX_via_STLINK/mbedtls/include/mbedtls/config.h

mv .pio/libdeps/LEA_2400_AIO_TX_via_STLINK/mbedtls/library/common.h .pio/libdeps/LEA_2400_AIO_TX_via_STLINK/mbedtls/library/mbedtls_common.h
mv .pio/libdeps/LEA_2400_AIO_RX_via_STLINK/mbedtls/library/common.h .pio/libdeps/LEA_2400_AIO_RX_via_STLINK/mbedtls/library/mbedtls_common.h


