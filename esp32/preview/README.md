# Preview

Renders the display to a PNG on the host, so layout work does not need a flash
cycle and a four second panel refresh.

```
preview-esp             # writes preview.png
preview-esp out.png     # or somewhere else
```

Or directly, from this folder: `make run`.

## How it works

`GxEPD2_BW` and `GFXcanvas1` are both `Adafruit_GFX` subclasses, so drawing code
written against `Adafruit_GFX&` runs unchanged on either. The preview points it
at a 480x800 `GFXcanvas1` and writes the buffer out as a 1-bit PNG.

`shim/` supplies the handful of Arduino declarations `Adafruit_GFX` expects —
`Print`, `String`, the `pgm_read_*` macros, `yield()` — so the library compiles
against the host toolchain. `Adafruit_SPITFT` is not compiled, hence the empty
`Adafruit_I2CDevice.h` and `Adafruit_SPIDevice.h` stubs.

Adafruit_GFX itself is read from the PlatformIO dependency tree, so run
`build-esp` once before the first preview.

## Caveats

The canvas is not the panel. It gets the geometry, fonts and text metrics right,
which is what layout work needs, but says nothing about contrast, ghosting or
refresh behaviour. Check those on hardware.
