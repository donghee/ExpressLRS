#!/bin/sh

cd "$(dirname "$0")/.."

DEV_HOSTDIR=$(pwd)
DEV_DOCKERDIR=/workspaces/ExpressLRS

CONTAINER_NAME=ExpressLRS

echo "If container $CONTAINER_NAME is already running. Stop and remove the container."
docker stop $CONTAINER_NAME; docker rm $CONTAINER_NAME
docker run -d -it --device=/dev/bus/usb:/dev/bus/usb -v /dev:/dev -v $DEV_HOSTDIR:$DEV_DOCKERDIR --name $CONTAINER_NAME -t ghcr.io/donghee/expresslrs:latest
