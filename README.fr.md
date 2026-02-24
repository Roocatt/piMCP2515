# piMCP2515

[![Pico SDK Build](https://github.com/Roocatt/piMCP2515/actions/workflows/pico-sdk.yml/badge.svg)](https://github.com/Roocatt/piMCP2515/actions/workflows/pico-sdk.yml)
[![Linux SPI Build](https://github.com/Roocatt/piMCP2515/actions/workflows/linux-spi.yml/badge.svg)](https://github.com/Roocatt/piMCP2515/actions/workflows/linux-spi.yml)
[![CodeQL](https://github.com/Roocatt/piMCP2515/actions/workflows/github-code-scanning/codeql/badge.svg)](https://github.com/Roocatt/piMCP2515/actions/workflows/github-code-scanning/codeql)
![GitHub Licence](https://img.shields.io/github/license/Roocatt/piMCP2515?label=licence)

<a href="README.md">🇬🇧 English</a> |
<a href="README.sv.md">🇸🇪 Svenska</a> |
<a href="README.fr.md">🇫🇷 Français</a>

piMCP2515 est une bibliothèque pour communiquer avec un MCP2515
via SPI. Ça marche avec les Raspberry Pi Pico, et les appareils
Raspberry Pi qui fonctionnent sous FreeBSD, NetBSD ou Linux.

C'est un travail en cours, et n'est pas testé présentement. Il
n'y a aucune garantie que ça marche maintenant, mais je travaille
sur ça et je veux en finir bientôt.

Le travail et les objectifs sont décrits en anglais dans le fichier
`README.md`, et aussi dans les commentaires `TODO` dans les fichiers
sources.

## Systèmes pris en charge

Les Raspberry Pi Pico sont pris en charge, et aussi les appareils
Raspberry Pi qui fonctionnent sous FreeBSD, NetBSD ou basés sur Linux.

C'est possible que piMCP2515 fonctionne sur d'autres SBC fonctionnant
sous BSD ou Linux parce que le fonctionnement interne de cette
bibliothèque est assez générique, mais ça n'est pas garanti.

Parce qu'OpenBSD ne semble pas prendre en charge les fonctionnalités
dont piMCP2515 a besoin avec SPI, piMCP2515 ne prend pas en charge
OpenBSD. Si ça change dans le futur, la prise en charge d’OpenBSD
sera prioritaire.

## Documentation

Il y a une documentation générée automatiquement sur
[GitHub Pages](https://roocatt.github.io/piMCP2515/). Il y a aussi la 
cible `docgenerate` dans le Makefile pour générer les pages man et la
même documentation HTML qui sont disponibles sur GitHub Pages. Si vous
changez la configuration dans `Doxyfile.in`, il est possible de générer
quelque chose d'autre pour la documentation. Pour générer la
documentation automatiquement, Doxygen doit être installé.

## Exemples

Il y a plusieurs exemples dans le répertoire d'`examples`. C'est un
bon point à commencer avec cette bibliothèque.

## License

Ce projet est sous licence ISC.