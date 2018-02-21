BUILD := build
SHELL := /bin/bash

default: all

$(BUILD):
	mkdir -p $(BUILD)

core/config.h:
	cp core/config.h.template core/config.h

all: $(BUILD) gitdeps core/config.h seed
	cd $(BUILD) && cmake ../ -DDEBUG_UART_FALLBACK=ON
	cd $(BUILD) && make

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

clean:
	rm -rf $(BUILD)

veryclean: clean
	rm -rf gitdeps
