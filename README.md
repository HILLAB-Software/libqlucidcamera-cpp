# libqlucidcamera-cpp

`libqlucidcamera-cpp` is a C++ library for controlling and streaming from LUCID cameras.
It provides camera discovery/connection, runtime control for acquisition parameters, and frame delivery via Qt signals.

## Features

- Camera discovery and connection management
- Stream start/stop and frame grabbing with OpenCV (`cv::Mat`)
- Exposure, gain, frame-rate, ROI, and pixel-format control
- Trigger configuration (mode/source/activation)
- PTP-related controls and status access
- Qt/QML-friendly API (`QObject`, `Q_PROPERTY`, signals/slots)

## Requirements

- C++17 compiler
- CMake 3.16 or higher
- Qt 6.8+ (`Core`, `Qml`, `Concurrent`)
- OpenCV 4.0+
- LUCID Arena SDK

## Arena SDK setup

If your Arena SDK package is not discoverable by CMake, set one of these environment variables:

```bash
export ARENA_SDK_ROOT=/path/to/ArenaSDK
# or
export ARENA_SDK_DIR=/path/to/ArenaSDK
```

The SDK root should contain folders similar to `include/` and `lib64/`.

## Building the library

### Linux (recommended with presets)

From the project root:

```bash
cmake --preset qt610-debug
cmake --build --preset build-qt610-debug
```

Release build:

```bash
cmake --preset qt610-release
cmake --build --preset build-qt610-release
```

If you prefer environment-driven Qt path presets:

```bash
export QT_ROOT=/path/to/Qt/6.x.x/gcc_64
cmake --preset qt-env-debug
cmake --build --preset build-qt-env-debug
```

### Generic CMake build

```bash
mkdir -p build/manual
cmake -S . -B build/manual -DCMAKE_BUILD_TYPE=Release
cmake --build build/manual -j
```

## Installing

Install into a custom prefix:

```bash
cmake --install build/qt610-debug --prefix build/qt610-debug/stage
```

Installed CMake package files are generated under:

- `lib/cmake/qlucidcamera/qlucidcameraConfig.cmake`
- `lib/cmake/qlucidcamera/qlucidcameraTargets.cmake`

## Output

Debug preset output example:

- Shared library: `build/qt610-debug/libqlucidcamerad.so`

Release preset output example:

- Shared library: `build/qt610-release/libqlucidcamera.so`

## License

This project is licensed under the GNU General Public License v3.0.
