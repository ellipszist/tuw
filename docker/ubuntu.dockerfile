# Building workflow for distributions using glibc.
#
# 1. Use this docker file to build the executable.
#    docker build -t tuw_ubuntu -f docker/ubuntu.dockerfile ./
#
# 2. Run the built image.
#    docker run --name tuw_ubuntu tuw_ubuntu
#
# 3. Use "docker cp" to get the built executable.
#    docker cp tuw_ubuntu:/Tuw/build/Release/Tuw ./
#
# Notes:
#   -You can use buildx for cross compiling
#    sudo apt install -y qemu-user-static binfmt-support
#    docker buildx build --platform linux/arm64 -t tuw_ubuntu -f docker/ubuntu.dockerfile ./
#
#   -You can run test.sh with build-arg
#    docker build --build-arg TEST=true -t tuw_ubuntu -f docker/ubuntu.dockerfile ./

# Base image
FROM ubuntu:20.04

# Run test.sh when true
ARG TEST=false

# Install packages
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
            ca-certificates build-essential \
            libgtk-3-dev git python3-pip && \
    if [ "$TEST" = "true" ]; then \
        apt-get install -y --no-install-recommends dbus-x11 xdg-utils xvfb firefox; \
    fi && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Install meson
RUN pip3 install meson==1.3.1 ninja==1.11.1

# Clone the repo
COPY . /Tuw

# Build
ENV XDG_CURRENT_DESKTOP=GNOME
WORKDIR /Tuw/shell_scripts
RUN if [ "$TEST" = "true" ]; then \
        gio mime x-scheme-handler/https firefox.desktop && \
        xvfb-run ./test.sh; \
    else \
        ./build.sh; \
    fi
