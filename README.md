# piMCP2515

[![Pico SDK Build](https://github.com/Roocatt/piMCP2515/actions/workflows/pico-sdk.yml/badge.svg)](https://github.com/Roocatt/piMCP2515/actions/workflows/pico-sdk.yml)
[![Linux SPI Build](https://github.com/Roocatt/piMCP2515/actions/workflows/linux-spi.yml/badge.svg)](https://github.com/Roocatt/piMCP2515/actions/workflows/linux-spi.yml)
[![CodeQL](https://github.com/Roocatt/piMCP2515/actions/workflows/github-code-scanning/codeql/badge.svg)](https://github.com/Roocatt/piMCP2515/actions/workflows/github-code-scanning/codeql)
![GitHub Licence](https://img.shields.io/github/license/Roocatt/piMCP2515?label=licence)

<a href="README.md">🇬🇧 English</a> |
<a href="README.sv.md">🇸🇪 Svenska</a> |
<a href="README.fr.md">🇫🇷 Français</a>

A library for interacting with the MCP2515 CAN bus controller via SPI.
This library supports the Raspberry Pi Pico, as well as regular
Raspberry Pi devices running FreeBSD, NetBSD or Linux based operating
systems.

This is a work in progress which is incomplete and has not been
tested. It is not yet usable, but I am working on it though, and I
hope to have this complete soon.

In addition to the 'Goals' section detailing larger scale targets for
this project, there are numerous `TODO` comments outlining things that
need doing. These will make varying degrees of sense to anyone else
reading the code as many are intended slightly more as personal notes.

## Development Goals

- [ ] Supported targets:
  - [x] Pi Pico
    - [x] Feature Complete (Compiles)
    - [x] Run on Device
    - [x] Tested
  - [ ] Regular Raspberry Pi devices
    - [ ] With Linux
      - [x] Feature Complete (Compiles)
      - [ ] Run on Device
      - [ ] Tested
    - [ ] With FreeBSD
        - [x] Feature Complete (Compiles)
        - [ ] Run on Device
        - [ ] Tested
    - [ ] With NetBSD
        - [x] Feature Complete (Compiles)
        - [ ] Run on Device
        - [ ] Tested
- [ ] Unit testing
- [x] Easy cross-compiling
- [ ] Detailed documentation
  - [ ] Github wiki
    - [ ] Compiling/Setup
    - [ ] Quickstart guide
    - [ ] General Tutorials
  - [x] Examples
  - [x] Doxygen API docs

## Supported Platforms

The Raspberry Pi Pico is supported, as are FreeBSD, NetBSD and Linux
on standard Raspberry Pi devices.

The internal implementations for BSDs and Linux are relatively generic
in nature, and it should be possible for it to work on other similar
devices running one of the supported OS options. This is of course not
guaranteed to work or behave as expected.

OpenBSD seems to lack the relevant SPI drivers (at least at time of
writing) and as such it is not currently supported. Should this change
in future OpenBSD releases, or should a method of adding support be
found, then adding OpenBSD support will be a priority.

## Documentation

There is automatically generated API documentation available on
[GitHub Pages](https://roocatt.github.io/piMCP2515/). There is also a
`docgenerate` Makefile target to generate some documentation in the
`doxygen` directory which is configured by default to generate man
pages as well as the same HTML documentation seen on GitHub pages.
Automated documentation generation is done using Doxygen, which must
be installed to work.

## Examples

There are several examples with detailed comments in the `examples`
directory. This is a great place to start with this library.

## Known Issues

None at the moment! 🙆‍♀️

## Licence
This project is available under the ISC licence.