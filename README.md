# sm_tracer

`sm_tracer` is a cross-platform GUI for decoding and displaying trace frames produced by a hierarchical state machine system. It is built on [QP/C](https://www.state-machine.com/), [LVGL](https://lvgl.io/) v8.3.3, SDL2, and libserialport.

## Features

- Decode framed trace messages from the serial pipeline
- Map `RecID` values to human-readable formats through JSON config files
- Display parsed trace output in a desktop GUI
- Build with standard `CMakeLists.txt` + `CMakePresets.json` + `toolchain.cmake`

## Prerequisites

- `git`
- `gcc` or another compatible C compiler
- `cmake` >= 3.20
- `ninja`

## Quick Start

### 1. Clone with submodules

```bash
git clone --recurse-submodules <repo-url>
```

If the repository was already cloned without submodules:

```bash
git submodule update --init --recursive
```

### 2. Configure

```bash
cmake --preset configure
```

### 3. Build

```bash
cmake --build --preset build-debug
```

### 4. Run

```bash
cmake --build --preset run-debug
```

Typical executable locations:

- Linux: `build/bin/Debug/sm_tracer`
- Windows: `build/bin/Debug/sm_tracer.exe`

## Protocol Frame Format

The trace transport uses a framed byte stream. A complete frame is represented as:

```text
[0x7E] [seq.] [RecID] [pldLen] [payload...] [chksum] [0x7E]
```

### Fields

- `0x7E`: frame boundary flag
- `seq.`: rolling sequence number for the emitted trace frame
- `RecID`: record identifier used to select the parser rule from the JSON config
- `pldLen`: payload length in bytes
- `payload...`: raw payload bytes interpreted according to the `RecID` rule
- `chksum`: checksum byte; the parser validates it so that payload bytes plus checksum sum to `0xFF`
- trailing `0x7E`: end flag

### Parsing Flow

1. The frame parser removes flags and validates the checksum.
2. The application reads `RecID` and looks up the matching token definition.
3. `pldLen` determines how many payload bytes are consumed.
4. The payload is decoded according to the configured argument types.
5. The GUI renders the final formatted message.

If a `RecID` is not configured, the GUI falls back to a raw summary view such as `[seq][RecID][pldLen][????]`.

## JSON Configuration

The protocol layer is designed to work together with a JSON-based token description file.

In practice, the loader accepts a **JSONC-style** file:

- `// ...` and `/* ... */` comments are allowed in the source file
- comments are stripped before `cJSON_Parse(...)`
- trailing commas must not be used

### RX Token Rules

Each `RX_tokens` entry binds one `RecID` to:

- a display `format`
- an `args` list describing how to decode the payload bytes

Supported argument names include:

- Decimal: `UINT8`, `UINT16`, `UINT32`, `UINT64`, `INT8`, `INT16`, `INT32`, `INT64`
- Float: `F32`, `F64`
- Hex: `HEX8`, `HEX16`, `HEX32`, `HEX64`
- Binary: `BIN8`, `BIN16`, `BIN32`, `BIN64`
- String: `STRING`

> `STRING` must be the last argument because it consumes the remaining payload bytes.

### Example

```jsonc
// !NOTE: Never leave a trailing comma!!! ====================================

//============================================================================
// SM_Trace parser.
{
  // META...
  "meta": {
    "version": "0.0.0.1",
    "device": "SM_Tracer"
  },

  // @brief Receive parser.
  //
  // @enum Dec: UINT8 UINT16 UINT32 UINT64
  //            INT8  INT16  INT32  INT64
  //
  //       Hex: HEX8 HEX16 HEX32 HEX64
  //
  //       Bin: BIN8 BIN16 BIN32 BIN64
  //
  //       String: STRING (Must be the last one arg.)
  "RX_tokens": [
    //========================================================================
    // RESERVED.
    // Assertion infomation...
    {
      "RecID": 0,
      "format": "==[ASSERTION]==[Label][%s]==[Module][%s]==\n",
      "args": ["INT32", "STRING"]
    },
    //========================================================================
    // Normal...
    {
      "RecID": 10,
      "format": "==TimeStamp:%s==ledOff==\n",
      "args": ["UINT32"]
    },
    {
      "RecID": 11,
      "format": "==TimeStamp:%s==ledOn ==\n",
      "args": ["UINT32"]
    }
  ]
}
```

### How `RecID` and Payload Work Together

For a frame like:

```text
[0x7E] [005] [010] [004] [payload...] [chksum] [0x7E]
```

- `seq = 5`
- `RecID = 10`
- `pldLen = 4`
- the loader finds the rule with `"RecID": 10`
- `args: ["UINT32"]` tells the parser to decode 4 payload bytes as one unsigned 32-bit integer
- `format: "==TimeStamp:%s==ledOff==\n"` defines the final rendered string

## Build System Layout

The project now follows the standard desktop CMake split:

- `CMakeLists.txt`: project structure, sources, targets, link rules
- `CMakePresets.json`: standard configure/build/run presets
- `toolchain.cmake`: compiler and warning flag selection

Common commands:

```bash
cmake --preset configure
cmake --build --preset build-debug
cmake --build --preset build-release
cmake --build --preset run-debug
```

## Project Structure

- `application/` - application logic, parsers, and managers
- `bsp/` - SDL2, LVGL, and serial-port board support glue
- `ports/` - QP/C port layer
- `include/` - shared headers for the local framework code
- `src/` - local QP/C source tree used by the build
- `3rd_party/` - external dependencies and prebuilt runtime libraries
- `qm_model/` - QM model sources
- `doc/` - project documentation assets

## Notes

- Prebuilt SDL2 and libserialport binaries are loaded from `3rd_party/libs/`
- Generated or model-derived files should be treated carefully and not rewritten casually
- If you change the JSON token contract, update this README together with the parser behavior
