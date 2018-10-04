#!/bin/bash

set -xe

rm -rf "build/core/fk-core-*"

pushd build
make -j4
ls -alh
pushd core
arm-none-eabi-objdump -h fk-core-amazon.elf
dd if=/dev/zero of=config.bin bs=1 count=1024
arm-none-eabi-objdump -j .config -s fk-core-amazon.elf
arm-none-eabi-objcopy --update-section .config=config.bin fk-core-amazon.elf
arm-none-eabi-objdump -j .config -s fk-core-amazon.elf
rm -f fk-core-amazon.bin
arm-none-eabi-objcopy -O binary fk-core-amazon.elf fk-core-amazon.bin
popd
popd
