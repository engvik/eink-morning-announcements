# Builds the display code for the host so layouts can be checked without
# flashing. Adafruit_GFX comes from the PlatformIO dependency tree, compiled
# against the shim in shim/ instead of the Arduino core.
GFX := ../.pio/libdeps/lolin_d32_pro/Adafruit GFX Library
JSON := ../.pio/libdeps/lolin_d32_pro/Arduino_JSON/src

CXX ?= c++
# ARDUINO must be >= 100 or Adafruit_GFX.h reaches for the pre-1.0 WProgram.h.
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -DARDUINO=10805 -DPREVIEW_HOST \
            -Ishim -I"$(GFX)" -I"$(JSON)" -I. -I../src \
            -DCFG_DISPLAY_PADDING_TOP='"$(DISPLAY_PADDING_TOP)"' \
            -DCFG_DISPLAY_PADDING_RIGHT='"$(DISPLAY_PADDING_RIGHT)"' \
            -DCFG_DISPLAY_PADDING_BOTTOM='"$(DISPLAY_PADDING_BOTTOM)"' \
            -DCFG_DISPLAY_PADDING_LEFT='"$(DISPLAY_PADDING_LEFT)"'
OUT := preview

# The insets are compiled in, so rewrite the stamp when they change to force a rebuild.
INSETS := $(DISPLAY_PADDING_TOP):$(DISPLAY_PADDING_RIGHT):$(DISPLAY_PADDING_BOTTOM):$(DISPLAY_PADDING_LEFT)
$(shell echo '$(INSETS)' | cmp -s - .insets || echo '$(INSETS)' > .insets)

# The library path contains a space, so it stays inside the recipe rather than
# in a prerequisite list.
DEPS := main.cpp png.h shim/Arduino.h shim/Print.h .insets \
        ../src/ui/text.cpp ../src/ui/text.h ../src/ui/theme.h \
        ../src/icons/met_icons_black_24x24.cpp ../src/icons/met_icons_black_44x44.cpp \
        ../src/icons/symbols.cpp ../src/icons/symbols.h ../src/icons/symbols_table.h \
        ../src/ui/panel.cpp ../src/ui/panel.h ../src/ui/decode.cpp ../src/ui/datetime.cpp \


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
		../src/icons/symbols.cpp \
		../src/ui/panel.cpp ../src/ui/decode.cpp ../src/ui/datetime.cpp \
		"$(JSON)/cjson/cJSON.c" \
		"$(GFX)/Adafruit_GFX.cpp"

run: $(OUT)
	./$(OUT)

clean:
	rm -f $(OUT) *.png .insets
