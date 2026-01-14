# piMCP2515

A library for interacting with the MCP2515 CAN bus controller via SPI.
This library supports the Raspberry Pi Pico, as well as regular
Raspberry Pi devices running BSD and Linux based operating systems.

This is an early work in progress which is incomplete and has not been
tested. It is not yet usable, but I am working on it though, and I
hope to have this complete soon.

In addition to the 'Goals' section detailing larger scale targets for
this project, there are numerous `TODO` comments outlining things that
need doing. These will make varying degrees of sense to anyone else
reading the code as many are intended slightly more as personal notes.

## Goals

- [ ] Finish building and testing the initial code
- [ ] Support for the Pi Pico
- [ ] Support for regular Raspberry Pi devices
  - [ ] With Linux
  - [ ] With BSD
- [ ] Unit testing
- [x] Easy cross-compiling
- [ ] Detailed documentation
  - [ ] Github wiki
  - [ ] Examples
  - [x] Doxygen API docs

## Documentation

There is automatically generated API documentation available on
[GitHub Pages](https://roocatt.github.io/piMCP2515/). There is also a
`docgenerate` Makefile target to generate some documentation in the
`doxygen` directory which is configured by default to generate man
pages as well as the same HTML documentation seen on GitHub pages.
Automated documentation generation is done using Doxygen, which must
be installed to work.

## Licence
This project is available under the ISC licence.