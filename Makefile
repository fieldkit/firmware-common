BUILD=build

default: all

$(BUILD):
	mkdir -p $(BUILD)

all: $(BUILD) gitdeps
	cd $(BUILD) && cmake ../
	cd $(BUILD) && make

gitdeps:
	simple-deps --config core/arduino-libraries
	simple-deps --config module/arduino-libraries

clean:
	rm -rf $(BUILD) gitdeps
