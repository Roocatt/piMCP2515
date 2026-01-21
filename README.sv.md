# piMCP2515

[![Pico SDK Build](https://github.com/Roocatt/piMCP2515/actions/workflows/pico-sdk.yml/badge.svg)](https://github.com/Roocatt/piMCP2515/actions/workflows/pico-sdk.yml)
[![Linux SPI Build](https://github.com/Roocatt/piMCP2515/actions/workflows/linux-spi.yml/badge.svg)](https://github.com/Roocatt/piMCP2515/actions/workflows/linux-spi.yml)
![GitHub Licence](https://img.shields.io/github/license/Roocatt/piMCP2515?label=licence)


<a href="README.md">🇬🇧 English</a> |
<a href="README.sv.md">🇸🇪 Svenska</a>

piMCP2515 är ett bibliotek för att kommunicera med en MCP2515
CAN-styrenhet via SPI. Det här biblioteket stöder Raspberry Pi Pico,
och Raspberry Pi-enheter med FreeBSD, NetBSD eller Linux-baserade
operativsystem.

Det här är ett pågående arbete och är inte testat, men jag jobbar på
det och jag hoppas att få det färdigt snart.

Arbete och mål är beskrivna i den engelska `README.md`-filen, och på
flera `TODO`-kommentarer i källfilerna.

## System som stöds

Raspberry Pi Pico stöds, samt Raspberry Pi-enheter som kör FreeBSD,
NetBSD eller Linux-baserade operativsystem.

De interna implementeringarna för BSD och Linux är ganska generiska. Så
det är möjligt att piMCP2515 fungerar på andra liknande SBC-enheter om
de kör ett av de operativsystem som stöds. Det finns ingen garanti att
det här kommer att fungera eller fungera som förväntat.

Eftersom OpenBSD inte verkar stödja SPI för vad det här biblioteket
behöver, stöder piMCP2515 inte OpenBSD. Om detta ändras i framtiden,
stöd för OpenBSD kommer att vara prioriterat.

## Dokumentation

Det finns API-dokumentation på engelska på
[GitHub Pages](https://roocatt.github.io/piMCP2515/) som genereras
automatiskt med Doxygen. Det finns också ett Makefile-mål som
`docgenerate` för att generera dokumentation. Det genereras HTML och
man-sidor men om man ändrar `Doxyfile.in` kan det genereras flera andra
format som man vill. För att detta ska fungera måste Doxygen
installeras.

## Licens

Det här projektet är tillgängligt under ISC-licensen.