# Builds the display code for the host so layouts can be checked without
# flashing. Adafruit_GFX comes from the PlatformIO dependency tree, compiled
# against the shim in shim/ instead of the Arduino core.
GFX := ../.pio/libdeps/lolin_d32_pro/Adafruit GFX Library

CXX ?= c++
# ARDUINO must be >= 100 or Adafruit_GFX.h reaches for the pre-1.0 WProgram.h.
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -DARDUINO=10805 -DPREVIEW_HOST \
            -Ishim -I"$(GFX)" -I. -I../src
OUT := preview
OUTPUT ?= preview.png

# The library path contains a space, so it stays inside the recipe rather than
# in a prerequisite list.
DEPS := main.cpp png.h shim/Arduino.h shim/Print.h \
        ../src/ui/text.cpp ../src/ui/text.h ../src/ui/theme.h \
        ../src/icons/met_icons_black_24x24.cpp ../src/icons/met_icons_black_44x44.cpp

.PHONY: all run clean

all: $(OUT)

$(OUT): $(DEPS)
	@test -d "$(GFX)" || { \
		echo "Adafruit GFX not found. Run 'pio run -e lolin_d32_pro' in esp32/ first."; \
		exit 1; \
	}
	$(CXX) $(CXXFLAGS) -o $(OUT) main.cpp ../src/ui/text.cpp \
		../src/icons/met_icons_black_24x24.cpp \
		../src/icons/met_icons_black_44x44.cpp \
		"$(GFX)/Adafruit_GFX.cpp"

run: $(OUT)
	./$(OUT) $(OUTPUT)

clean:
	rm -f $(OUT) $(OUTPUT)
