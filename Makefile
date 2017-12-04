BUILD=build

default: all

$(BUILD):
	mkdir -p $(BUILD)

core/config.h:
	cp core/config.h.template core/config.h

all: $(BUILD) gitdeps core/config.h
	cd $(BUILD) && cmake ../
	cd $(BUILD) && make

gitdeps:
	simple-deps --config core/arduino-libraries
	simple-deps --config module/arduino-libraries

clean:
	rm -rf $(BUILD)

veryclean: clean
	rm -rf gitdeps
