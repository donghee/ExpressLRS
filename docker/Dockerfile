# 1. Ubuntu 기반 이미지 사용
FROM ubuntu:24.04

# 2. 필수 패키지 설치
RUN apt-get update && \
    apt-get install -y \
        python3 \
        python3-pip \
        git \
        curl \
        stlink-tools \
        libusb-1.0.0-dev \
        build-essential \
        && rm -rf /var/lib/apt/lists/*

# 3. PlatformIO 설치
RUN pip3 install --no-cache-dir platformio --break-system-packages --user
ENV HOME=/root
ENV PATH="$HOME/.local/bin:$PATH"

# 4. 작업 디렉토리 지정
WORKDIR /workspaces/ExpressLRS

# 5. 기본 커맨드: bash
COPY ./docker/entrypoint.sh /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
CMD ["bash"]
