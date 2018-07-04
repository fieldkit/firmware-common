BUILD := build
SHELL := /bin/bash

default: all

core/config.h:
	cp core/config.h.template core/config.h

all: $(BUILD)/Makefile
	cd $(BUILD) && make all

$(BUILD)/Makefile: gitdeps core/config.h seed
	mkdir -p $(BUILD)
	cd $(BUILD) && cmake ../ -DDEBUG_UART_FALLBACK=ON -DDISABLE_RADIO=ON

doc: $(BUILD)/Makefile
	cd $(BUILD) && make doc

test: $(BUILD)/Makefile
	cd $(BUILD) && env GTEST_COLOR=1 make testcommon test ARGS=-VV

seed:
	echo "// Generated before compile time to seed the RNG." > core/seed.h
	echo "" >> core/seed.h
	echo "#define RANDOM_SEED $$RANDOM" >> core/seed.h

gitdeps:
	simple-deps --config core/arduino-libraries
	simple-deps --config module/arduino-libraries
	simple-deps --config test/mcu/arduino-libraries
	simple-deps --config test/linux/testcommon/arduino-libraries

clean:
	rm -rf $(BUILD)

veryclean: clean
	rm -rf gitdeps
