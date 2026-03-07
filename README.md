# sm_tracer

A cross-platform hierarchical state machine (HSM) tracer GUI built on [QP/C](https://www.state-machine.com/), [LVGL](https://lvgl.io/) v8.3.3, and SDL2.

## Prerequisites

- `git`
- `gcc` (or compatible C compiler)
- `cmake` ≥ 3.10
- `ninja`
- `python3`

## Quick Start (Linux — prebuilt `.so` included)

```bash
# Clone with submodules
git clone --recurse-submodules <repo-url>

# Build
python3 build.py build

# Run
./build/bin/sm_tracer
```

If you already cloned without `--recurse-submodules`:

```bash
python3 build.py setup
python3 build.py build
```

## Project Structure

- `application/` — Application layer (HSM logic, UI)
- `bsp/` — Board support package (SDL2/serial port abstraction)
- `ports/` — Platform ports (QP/C)
- `3rd_party/` — Third-party dependencies
  - `cJSON/` — JSON library (git submodule)
  - `lvgl/` — LVGL GUI library v8.3.3 (git submodule)
  - `sm_hsm/` — HSM framework (git submodule)
  - `libs/` — Prebuilt shared libraries (SDL2, libserialport)
