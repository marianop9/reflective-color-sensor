# Color Sensor Firmware

## Setup

Initialize FreeRTOS, pico-sdk submodules:

```sh
git submodule update --init
```

Configure CMake (run from root dir):

```sh
cmake -S . -B build
```

Build:

```sh
cmake --build .\build\
```