# Prebuilt Libraries

This directory contains prebuilt shared libraries for SDL2 and libserialport.

## Linux

Prebuilt `.so` files are already included in this repository:

- `SDL2/linux/` — SDL2 2.28.5 shared library for x86_64 Linux
- `libserialport/linux/` — libserialport 0.1.2 shared library for x86_64 Linux

No action needed for Linux users.

## Windows (Manual Setup Required)

Download SDL2 MinGW development package from:
https://github.com/libsdl-org/SDL/releases/tag/release-2.28.5

Extract and place files:

- `SDL2.dll` → `SDL2/windows/SDL2.dll`
- `lib/x64/libSDL2.dll.a` → `SDL2/windows/libSDL2.dll.a`
- `include/SDL2/` → `SDL2/windows/include/SDL2/`

For libserialport on Windows, compile from source or obtain prebuilt:

- `libserialport-0.dll` → `libserialport/windows/libserialport-0.dll`
- `libserialport.dll.a` → `libserialport/windows/libserialport.dll.a`
- `libserialport.h` → `libserialport/windows/include/libserialport.h`

## macOS (Manual Setup Required)

Install SDL2 via Homebrew (`brew install sdl2`) or download from SDL2 releases.
Place the shared library:

- `libSDL2.dylib` → `SDL2/macos/libSDL2.dylib`
- SDL2 headers → `SDL2/macos/include/SDL2/`

For libserialport on macOS, compile from source or install via Homebrew.
Place files:

- `libserialport.dylib` → `libserialport/macos/libserialport.dylib`
- `libserialport.h` → `libserialport/macos/include/libserialport.h`
