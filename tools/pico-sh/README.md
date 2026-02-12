# Pico-Sh

This is a testing application for the Raspberry Pi Pico. It acts as a 
sort of REPL-like shell over UART so that in developing piMCP2515, you
can test things more easily.

## Usage

This tool is only for the Pi Pico.

Build the core library for Pico first, then `cd` here and build with
CMake.

```shell
# Where $PI_MCP2515_PROJ is the root of this repository
cd $PI_MCP2515_PROJ
cmake -DUSE_PICO_LIB=1 .
make

cd tools/pico-sh
cmake .
make

# Then deploy piMCP2515-pico-sh.uf2 to a Pico device
```