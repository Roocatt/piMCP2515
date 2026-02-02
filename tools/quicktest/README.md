# Quicktest

Run through a series of library functions in loopback mode to test
functionality.

## Usage

This tool is only for the Pi Pico.

Build the core library for Pico first, then `cd` here and build with
CMake.

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