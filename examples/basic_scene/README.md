# Basic Scene Example

This example demonstrates how to use the **ESP32 3D Engine** component to render a 3D scene with multiple objects on an ESP32 board.

## What it does

- Creates 10 different 3D meshes (cylinders, cones, vases, toroids, prisms, stars, etc.)
- Uses both **revolution** and **extrusion** mesh generation
- Sets up a camera that automatically orbits around the scene
- Applies directional lighting with diffuse shading

## Supported hardware

| Target    | Display driver |
|-----------|---------------|
| ESP32-P4  | EK79007       |
| ESP32-S3  | ST7262        |

## How to use

### Set the target

```bash
idf.py set-target esp32p4   # or esp32s3
```

### Build and flash

```bash
idf.py build
idf.py -p PORT flash monitor
```

Replace `PORT` with the serial port of your board (e.g., `COM3` on Windows or `/dev/ttyUSB0` on Linux).

## Configuration

You can adjust rendering options via `idf.py menuconfig` under **ESP32 3D Engine - Graphics Library Options**:

- **Enable Parallel Graphics** — uses a thread pool for parallel scene update and rendering.
- **Enable Painter Algorithm** — sorts faces back-to-front for correct transparency-free rendering.
- **Enable Graphics Optimization** — enables per-face culling optimizations.
