# (lib)unarr changelog

## 1.0.1 - 2017-11-04
This is a bugfix release.

### Fixed
* Fixed typo in pkg-config.pc.cmake template

## 1.0.0 - 2017-09-22

### Added
* Cmake based build system for library builds
* Support for pkg-config (libunarr.pc)
* Windows compatible export header for DLL builds
* xz utils / libLZMA can be used as decoder for LZMA1 and XZ (LZMA2) compressed
ZIP archives.
* The internal LZMA1 decoder can be replaced with xz utils / libLZMA if present

### Changed
* LZMA SDK code was updated to version 17.01 beta
* 7z extraction support is currently broken due to LZMA SDK api changes.
* Unarr sample application (unarr-test) and its makefile
  (legacy unarr build system) have been moved to the [test](test) folder

### Fixed
* Various small bugfixes related to compiler warnings
