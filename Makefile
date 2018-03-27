BUILD := build
SHELL := /bin/bash

default: all

core/config.h:
	cp core/config.h.template core/config.h

all: $(BUILD)/Makefile
	cd $(BUILD) && make all doc

$(BUILD)/Makefile: gitdeps core/config.h seed
	mkdir -p $(BUILD)
	cd $(BUILD) && cmake ../ -DDEBUG_UART_FALLBACK=ON

doc: $(BUILD)/Makefile
	cd $(BUILD) && make doc

test: all
	cd $(BUILD) && env GTEST_COLOR=1 make test ARGS=-VV

seed: GIT_HASH=$(shell git log -1 --pretty=format:"%H")
seed:
	echo "// Generated before compile time to seed the RNG." > core/seed.h
	echo "" >> core/seed.h
	echo "#define RANDOM_SEED $$RANDOM" >> core/seed.h
	echo $(GIT_HASH)
	echo "#define FIRMWARE_GIT_HASH \"$(GIT_HASH)\"" >> core/seed.h

gitdeps:
	simple-deps --config core/arduino-libraries
	simple-deps --config module/arduino-libraries
	simple-deps --config test/mcu/arduino-libraries
	simple-deps --config test/linux/testcommon/arduino-libraries

clean:
	rm -rf $(BUILD)

veryclean: clean
	rm -rf gitdeps
