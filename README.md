# ESP32 3D Engine

A software 3D rendering engine for ESP32 microcontrollers. Renders complete 3D scenes with lighting directly on the MCU — no GPU required.

[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-%3E%3D5.4.1-red.svg)](https://docs.espressif.com/projects/esp-idf/)
[![Component Registry](https://components.espressif.com/components/andresragot/esp32_3d_engine/badge.svg)](https://components.espressif.com/components/andresragot/esp32_3d_engine)

## Author

[Andres Ragot (@andresragot)](https://github.com/andresragot)

## [Online Documentation (Doxygen)](https://andresragot.github.io/esp32-p4-library/)

---

## Features

- **Complete software 3D pipeline** — vertex transform, projection, clipping (Sutherland-Hodgman), rasterization, and frame buffer management, all running on the ESP32.
- **Scene graph** — hierarchical node/component tree with named lookup (`ID()` macro).
- **Mesh generation from 2D profiles:**
  - **Revolution meshes** — rotate a 2D contour around an axis (cylinders, cones, vases, toroids, spheres...).
  - **Extrusion meshes** — extrude a 2D polygon along a height (prisms, stars, letters...).
- **Camera system** — perspective projection, orbital animation, configurable FOV / aspect / clipping planes.
- **Directional lighting** — ambient + diffuse per-face shading in RGB565.
- **Painter's algorithm** — back-to-front depth sort for correct rendering without a Z-buffer.
- **Optional face-culling optimization** — per-face dot-product culling in local space to skip invisible geometry early.
- **Parallel rendering** — optional thread pool that runs scene update and render on separate cores (`std::jthread` + semaphores).
- **Double-buffered frame buffer** — supports RGB565, RGB888, and RGB8 color formats.
- **PSRAM allocator** — custom `std::allocator` that places large buffers in SPI RAM.
- **Pluggable display drivers** — abstract `DriverLCD` base class; swap drivers at compile time:

| Target | Driver | Interface |
|---|---|---|
| ESP32-P4 | EK79007 | MIPI-DSI |
| ESP32-S3 | ST7262 | RGB parallel |
| ESP32-S3 | ST7789 | SPI |

## Supported Targets

| ESP32 | ESP32-S2 | ESP32-S3 | ESP32-C3 | ESP32-P4 |
|:---:|:---:|:---:|:---:|:---:|
| * | * | * | * | * |

> Primary development and testing is on **ESP32-P4** and **ESP32-S3**.

---

## Quick Start (ESP-IDF Component Manager)

The simplest way to use the engine is via the [ESP Component Registry](https://components.espressif.com/):

**1. Add the dependency** to your project's `main/idf_component.yml`:

```yaml
dependencies:
  andresragot/esp32_3d_engine: "^1.0.0"
  andresragot/driver_ek79007: "^1.0.0"   # for ESP32-P4
  # andresragot/driver_st7262: "^1.0.0"  # for ESP32-S3 (RGB)
  # andresragot/driver_st7789: "^1.0.0"  # for ESP32-S3 (SPI)
```

**2. Build:**

```bash
idf.py set-target esp32p4   # or esp32s3
idf.py build
idf.py -p PORT flash monitor
```

The component manager downloads the engine, GLM, and all display drivers automatically.

### Use from source (this repo)

```bash
git clone https://github.com/andresragot/esp32-p4-library.git
cd esp32-p4-library/p4-graphics
idf.py set-target esp32p4
idf.py build
idf.py -p PORT flash monitor
```

---

## Example

A ready-to-build example is included in [`p4-graphics/examples/basic_scene/`](p4-graphics/examples/basic_scene/).

It creates 10 different 3D objects (revolution + extrusion), sets up an orbiting camera with directional lighting, and renders the scene on the display.

```bash
cd p4-graphics/examples/basic_scene
idf.py set-target esp32p4
idf.py build
idf.py -p PORT flash monitor
```

### Minimal code

```cpp
#include "Renderer.hpp"
#include "Scene.hpp"
#include "RevolutionMesh.hpp"
#include "ExtrudeMesh.hpp"
#include "Id.hpp"
#include "Light.hpp"

#ifdef CONFIG_IDF_TARGET_ESP32P4
#include "driver_ek79007.hpp"
#elif CONFIG_IDF_TARGET_ESP32S3
#include "Driver_ST7262.hpp"
#endif

using namespace Ragot;

extern "C" void app_main(void)
{
    // Camera
    Camera camera(320.f / 240.f);
    camera.set_location(glm::vec3(0.f, 0.f, -15.f));
    camera.set_target(glm::vec3(0.f, 0.f, 0.f));

    // Scene
    Scene scene(&camera);

    // Create a cylinder by revolving a 2D profile
    std::vector<coordinates_t> profile = {
        {1.0f, 0.0f},
        {1.0f, 2.0f},
        {0.0f, 2.0f}
    };
    mesh_info_t info(profile, RENDER_REVOLUTION);
    auto cylinder = std::make_shared<RevolutionMesh>(info, camera);
    cylinder->set_color(0xB345);
    scene.add_node(cylinder, ID(CYLINDER));

    // Create a cube by extruding a square
    std::vector<coordinates_t> square = {
        {0.f, 0.f}, {1.f, 0.f},
        {1.f, 1.f}, {0.f, 1.f}, {0.f, 0.f}
    };
    mesh_info_t cube_info(square, RENDER_EXTRUDE);
    auto cube = std::make_shared<ExtrudeMesh>(cube_info, camera);
    cube->set_position(glm::vec3(3.f, 0.f, 0.f));
    cube->set_color(0x07FF);
    scene.add_node(cube, ID(CUBE));

    // Display driver + renderer
#ifdef CONFIG_IDF_TARGET_ESP32P4
    DriverEK79007 display;
#elif CONFIG_IDF_TARGET_ESP32S3
    Driver_ST7262 display;
#endif

    Renderer renderer(320, 240, display);
    renderer.set_scene(&scene);
    renderer.init();
    renderer.set_light(DirectionalLight(
        glm::vec3(-0.4f, 0.6f, 0.8f), 0.50f, 0.80f
    ));

    scene.start();
    renderer.start();

    // Main loop (non-parallel mode)
    while (true) {
        scene.update(0);
        renderer.render();
    }
}
```

---

## Configuration

Via `idf.py menuconfig` > **ESP32 3D Engine - Graphics Library Options**:

| Option | Default | Description |
|---|:---:|---|
| `GRAPHICS_PARALLEL_ENABLED` | off | Run scene update and render on separate threads (recommended for dual-core chips). |
| `GRAPHICS_PAINTER_ALGO_ENABLED` | off | Painter's algorithm: sort all faces back-to-front before rasterization. |
| `GRAPHICS_OPTIMIZATION_ENABLED` | off | Per-face local-space culling to skip geometry not facing the camera. |

---

## Architecture

```
+--------------------------------------------------+
|                   app_main()                     |
+-------------+------------------------------------+
|   Scene     |            Renderer                |
|  +-------+  |  +-----------+  +--------------+   |
|  | Node  |  |  | Camera    |  | Rasterizer   |   |
|  |  - Mesh| |  |(view/proj |  |(fill tri/quad|   |
|  |  - ... |  |  | matrices)|  |  + clipping) |   |
|  +-------+  |  +-----------+  +------+-------+   |
|             |                        |            |
|             |                +-------v--------+   |
|             |                |  FrameBuffer   |   |
|             |                |  (double-buf)  |   |
|             |                +-------+--------+   |
|             |                        |            |
|             |                +-------v--------+   |
|             |                |   DriverLCD    |   |
|             |                |  (EK79007 /    |   |
|             |                |   ST7262 /     |   |
|             |                |   ST7789)      |   |
|             |                +----------------+   |
+-------------+------------------------------------+
```

### Core classes

| Class | Responsibility |
|---|---|
| `Scene` | Owns the node tree, camera, and `update()` logic. Collects meshes via BFS traversal. |
| `Node` | Base scene-graph node with parent/child hierarchy. |
| `Component` / `Transform` | Position, rotation, scale — inherited by `Mesh`. |
| `Mesh` | Abstract base: stores vertices + faces, calls `generate_vertices()` / `generate_faces()`. |
| `RevolutionMesh` | Generates geometry by revolving a 2D profile N slices around the Y axis. |
| `ExtrudeMesh` | Generates geometry by extruding a 2D polygon along Z. |
| `Camera` | Perspective projection (`glm::perspective`), view matrix (`glm::lookAt`), visibility checks. |
| `Renderer` | Orchestrates the pipeline: transform, clip, project, rasterize, send to display. |
| `Rasterizer` | Scanline fill for triangles and convex quads into the frame buffer. |
| `FrameBuffer<Color>` | Templated pixel buffer with optional double buffering. |
| `DriverLCD` | Abstract display interface — `send_frame_buffer()`, `set_pixel()`. |
| `Thread_Pool` | Singleton `std::jthread` pool with `std::stop_token` support for parallel mode. |
| `Logger` | Singleton logger with configurable verbosity (wraps `ESP_LOG` on device). |

---

## Project Structure

```
esp32-p4-library/
|-- README.md
|-- LICENSE
|-- docs/                          <- Doxygen HTML (GitHub Pages)
+-- p4-graphics/                   <- ESP-IDF component & main app
    |-- CMakeLists.txt             <- Top-level project CMake
    |-- idf_component.yml          <- Component registry metadata
    |-- Kconfig                    <- Menuconfig options
    |-- include/                   <- Public headers
    |   |-- Renderer.hpp
    |   |-- Scene.hpp
    |   |-- Camera.hpp
    |   |-- Mesh.hpp
    |   |-- RevolutionMesh.hpp
    |   |-- ExtrudeMesh.hpp
    |   |-- FrameBuffer.hpp
    |   |-- Rasterizer.hpp
    |   |-- Light.hpp
    |   |-- CommonTypes.hpp
    |   +-- ...
    |-- src/                       <- Implementation
    |   |-- Renderer.cpp
    |   |-- Scene.cpp
    |   |-- Camera.cpp
    |   +-- ...
    |-- main/                      <- Full demo application
    |   +-- main.cpp
    +-- examples/
        +-- basic_scene/           <- Standalone example project
            |-- CMakeLists.txt
            |-- sdkconfig.defaults
            |-- README.md
            +-- main/
                |-- CMakeLists.txt
                |-- idf_component.yml
                +-- main.cpp
```

---

## Dependencies

| Dependency | Purpose |
|---|---|
| [GLM](https://github.com/g-truc/glm) | Vector/matrix math (`glm::vec3`, `glm::mat4`, `glm::perspective`, ...) |
| [driver_lcd](https://github.com/andresragot/driver_lcd) | Abstract LCD base class |
| [driver_ek79007](https://github.com/andresragot/driver_ek79007) | MIPI-DSI driver for ESP32-P4 |
| [driver_ST7262](https://github.com/andresragot/driver_ST7262) | RGB parallel driver for ESP32-S3 |
| [driver_ST7789](https://github.com/andresragot/driver_ST7789) | SPI driver for ESP32-S3 |

All dependencies are resolved automatically by the ESP-IDF Component Manager.

## Contributing

Contributions are welcome. Please open an issue or submit a Pull Request.

## License

MIT — see [LICENSE](LICENSE) for details.