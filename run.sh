#!/usr/bin/env bash
set -e

rm -rf build/qemu-runner
mkdir -p build/qemu-runner

cp -v $OVMF_ROOT/FV/OVMF_VARS.fd build/qemu-runner/OVMF_VARS.fd
qemu-img create -f qcow2 build/qemu-runner/disk.img 5G

sudo -ES qemu-system-x86_64 \
  -drive if=pflash,format=raw,readonly=on,file=$OVMF_ROOT/FV/OVMF_CODE.fd \
  -drive if=pflash,format=raw,file=./build/qemu-runner/OVMF_VARS.fd \
  -drive file=./build/qemu-runner/disk.img,format=qcow2 \
  -cdrom ~/Downloads/isos/kubuntu-25.10-desktop-amd64.iso \
  -m 2048 \
  -display gtk
