BUILD_DIR = build
GENERATOR = Ninja

.PHONY: all configure demo clean rebuild

all: configure
	cmake --build $(BUILD_DIR)

configure:
	cmake -S . -B $(BUILD_DIR) -G $(GENERATOR)

demo: configure
	cmake --build $(BUILD_DIR) --target demo

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean all
