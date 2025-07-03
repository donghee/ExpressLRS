.PHONY: install-tools

setup:
	sudo apt-get update -y && sudo apt-get install gdb-multiarch libusb-1.0.0-dev -y
	pip3 install --no-cache-dir platformio --user --break-system-packages
	pip3 install -U pyocd --user --break-system-packages
	sudo cp docker/udev/*.rules /etc/udev/rules.d
	sudo udevadm control --reload
	sudo udevadm trigger

prebuild:
	sudo chown -R `id -un` src/.pio/build
	#git pull --rebase
	
build-tx: prebuild
	cd src; ./mbedtls_patch.sh; pio run -t clean -e LEA_2400_TX_via_STLINK && pio debug -e LEA_2400_TX_via_STLINK

build-rx: prebuild
	cd src; ./mbedtls_patch.sh; pio run -t clean -e LEA_2400_RX_via_STLINK && pio debug -e LEA_2400_RX_via_STLINK

build-aio-tx: prebuild
	cd src; ./mbedtls_patch.sh; pio run -t clean -e LEA_2400_AIO_RX_via_STLINK && pio debug -e LEA_2400_AIO_RX_via_STLINK

build-aio-rx: prebuild
	cd src; ./mbedtls_patch.sh; pio run -t clean -e LEA_2400_AIO_RX_via_STLINK && pio debug -e LEA_2400_AIO_RX_via_STLINK

build-docker-image:
	./docker/build.sh

build-using-container:
	docker exec ExpressLRS bash -c 'cd /workspaces/ExpressLRS/src; ./mbedtls_patch.sh; pio run -t clean -e LEA_2400_AIO_TX_via_STLINK'
	docker exec ExpressLRS bash -c 'cd /workspaces/ExpressLRS/src; ./mbedtls_patch.sh; pio debug -e LEA_2400_AIO_TX_via_STLINK'
