BUILD := build
SHELL := /bin/bash

default: all

$(BUILD):
	mkdir -p $(BUILD)

core/config.h:
	cp core/config.h.template core/config.h

all: $(BUILD) gitdeps core/config.h seed
	cd $(BUILD) && cmake ../
	cd $(BUILD) && make

seed:
	echo "// Generated before compile time to seed the RNG." > core/seed.h
	echo "" >> core/seed.h
	echo "#define RANDOM_SEED $$RANDOM" >> core/seed.h

gitdeps:
	simple-deps --config core/arduino-libraries
	simple-deps --config module/arduino-libraries

clean:
	rm -rf $(BUILD)

veryclean: clean
	rm -rf gitdeps
