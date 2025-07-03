.PHONY: setup

build-docker-image:
	./docker/build.sh

run-docker-container:
	./docker/run.sh

setup: build-docker-image
	sudo apt-get update -y && sudo apt-get install gdb-multiarch libusb-1.0.0-dev -y
	pip3 install --no-cache-dir platformio --user --break-system-packages
	ln -s ~/.platformio/penv/bin/platformio ~/.local/bin/pio
	pip3 install -U pyocd --user --break-system-packages
	sudo cp docker/udev/*.rules /etc/udev/rules.d
	sudo udevadm control --reload
	sudo udevadm trigger

prebuild:
	sudo chown -R `id -un` src/.pio/build
	# git pull --rebase --autostash
	
build-tx: prebuild
	cd src; ./mbedtls_patch.sh; pio run -t clean -e LEA_2400_TX_via_STLINK && pio debug -e LEA_2400_TX_via_STLINK

build-rx: prebuild
	cd src; ./mbedtls_patch.sh; pio run -t clean -e LEA_2400_RX_via_STLINK && pio debug -e LEA_2400_RX_via_STLINK

build-aio-tx: prebuild
	cd src; ./mbedtls_patch.sh; pio run -t clean -e LEA_2400_AIO_TX_via_STLINK && pio debug -e LEA_2400_AIO_TX_via_STLINK

build-aio-rx: prebuild
	cd src; ./mbedtls_patch.sh; pio run -t clean -e LEA_2400_AIO_RX_via_STLINK && pio debug -e LEA_2400_AIO_RX_via_STLINK

build-using-container:
	docker exec ExpressLRS bash -c 'cd /workspaces/ExpressLRS/src; ./mbedtls_patch.sh; pio run -t clean -e LEA_2400_AIO_TX_via_STLINK'
	docker exec ExpressLRS bash -c 'cd /workspaces/ExpressLRS/src; ./mbedtls_patch.sh; pio debug -e LEA_2400_AIO_TX_via_STLINK'

build: build-tx build-rx build-aio-tx build-aio-rx
	@echo 'Successfully built ExpressLRS TX and RX firmwares'

debug-aio-tx: build-aio-tx
	@echo 'pyocd gdbserver --persist -t stm32f479iihx -u 003800423433510837363934 -p 3333 -T 4444 # AIO TX STLINK-V3'
	cd src; gdb-multiarch -x .pioinit-aio-tx

debug-aio-rx: build-aio-rx
	@echo 'pyocd gdbserver --persist -t stm32f479iihx -u 004400353133510537363734 -p 3334 -T 4445 # AIO RX STLINK-V3'
	cd src; gdb-multiarch -x .pioinit-aio-rx
