# Quicktest

Run through a series of library functions in loopback mode to test
functionality.

## Usage

This tool is only for the Pi Pico.

Build the core library for Pico first, then `cd` here and build with
CMake.

At the end of running the test program, it will turn the onboard Pico
LED on solid if the test passed, or it will blink a number of times
then pause before repeating the blinks. The number of blinks will
correspond to a point in the test code to allow finding where it
failed without needing to use UART. This is however not perfect and
UART is a better way to see the results, but the blinking can help for
some quick checks.

The result LED blinking is 1 long blink for every 5, and one short for
each checkpoint after that.

```shell
# Where $PI_MCP2515_PROJ is the root of this repository
cd $PI_MCP2515_PROJ
cmake -DUSE_PICO_LIB=1 .
make

cd tools/quicktest
cmake .
make

# Then deploy piMCP2515-quicktest.uf2 to a Pico device
```