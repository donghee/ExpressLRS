cd "$(dirname "$0")/.."

docker build -t ghcr.io/donghee/expresslrs:latest -f docker/Dockerfile .
