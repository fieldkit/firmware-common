BUILD ?= $(abspath build)
SHELL := /bin/bash

default: all test

core/config.h:
	cp core/config.h.template core/config.h

all: $(BUILD)/Makefile
	cd $(BUILD) && make all

$(BUILD)/Makefile: gitdeps core/config.h seed
	mkdir -p $(BUILD)
	cd $(BUILD) && cmake ../

doc: $(BUILD)/Makefile
	cd $(BUILD) && make doc

test: $(BUILD)/Makefile
	cd $(BUILD) && env GTEST_OUTPUT="xml:$(BUILD)/tests.xml" GTEST_COLOR=1 make testcommon test ARGS="-VV"

seed:
	echo "// Generated before compile time to seed the RNG." > core/seed.h
	echo "" >> core/seed.h
	echo "#define RANDOM_SEED $$RANDOM" >> core/seed.h

gitdeps:
	simple-deps --config core/dependencies.sd
	simple-deps --config module/dependencies.sd
	simple-deps --config test/mcu/dependencies.sd
	simple-deps --config test/linux/testcommon/dependencies.sd

clean:
	rm -rf $(BUILD)

veryclean: clean
	rm -rf gitdeps
