# ESP32 3D Graphics Library
A lightweight 3D graphics rendering engine optimized for ESP32-P4 microcontrollers, with additional support for ESP32-S3 and desktop platforms.

<img alt="License" src="https://img.shields.io/badge/license-MIT-blue.svg">

## Author
[Andrés Ragot (@andresragot)](https://github.com/andresragot)

## [Online Documentation](https://andresragot.github.io/esp32-p4-library/)
👆 Click Here!

## Overview
This library provides a complete 3D graphics rendering pipeline designed specifically for resource-constrained ESP32 platforms. It implements a software rasterizer that efficiently renders 3D scenes while making optimal use of the limited resources available on microcontrollers.

## Features
* **Complete 3D Rendering Pipeline:** Software-based rasterization for ESP32 platforms
* **Scene Graph System:** Hierarchical node structure with component-based architecture
* **Mesh Support:** Load and render 3D meshes with various generation methods:
  * Revolution meshes (lathe objects)
  * Extrusion meshes
* **Camera System:** Perspective and orthographic projection support
* **Display Drivers:**
  * **ESP32-P4:** EK79007 display driver with MIPI DSI support
  * **ESP32-S3:** ST7789 display driver with SPI interface
* **Cross-Platform:** Works on both ESP32 microcontrollers and desktop environments
* **Multiple Rendering Modes:** Configure different rendering strategies based on your needs
* **Parallel Processing:** Optional multi-threaded rendering (configurable)
* **Memory Optimized:** Designed with resource constraints in mind
## Hardware Support
* **ESP32-P4:** Primary target platform with full feature support
* **ESP32-S3:** Supported with appropriate display driver
* **Desktop:** Supports development and testing on desktop environments
## Installation
### Prerequisites
* ESP-IDF v5.1 or later
* CMake 3.16+
### Quick Start
1. Clone the repository:
```console
git clone https://github.com/andresragot/esp32-p4-library.git
```

2. Add as a component to your ESP-IDF project:
```CMake
# In your project's CMakeLists.txt
set(EXTRA_COMPONENT_DIRS path/to/esp32-p4-library/p4-graphics)
```
3. Configure the library:
```console
idf.py menuconfig
# Navigate to Component Config > ESP32 Graphics Library
```
4. Build your project:
```console
idf.py build
```
5. Usage Example:
```cpp
#include "Renderer.hpp"
#include "Scene.hpp"
#include "Mesh.hpp"

using namespace Ragot;

extern "C" void app_main(void)
{
    // Initialize the renderer with display dimensions
    Renderer renderer(320, 240);
    renderer.init();
    
    // Create a scene
    Scene scene;
    
    // Create and configure a mesh
    vector < coordinates_t > coords_cube = { { 0 ,  0 } , { 0 ,  1 },
                                             { 1 ,  1 } , { 1 ,  0 }};

    mesh_info_t mesh_info_cube (coords_cube, RENDER_EXTRUDE);

    ExtrudeMesh    mesh_cube (mesh_info_cube, camera);

    // Add mesh to the scene
    scene.add_node(&mesh_cube, ID(CUBE));
    
    // Set the scene to be rendered
    renderer.set_scene(&scene);
    
    // Start rendering
    renderer.start();
    
    // Main loop
    while (true)
    {
        // Update scene animations, etc.
        scene.update(0.01f);
    }
}
```
6. Configuration Options
  The library provides various configuration options through ESP-IDF's Kconfig system:
  
  **GRAPHICS_PARALLEL_ENABLED:** Enable multi-threaded rendering for better performance
  **GRAPHICS_PAINTER_ALGO_ENABLED:** Enable the painter's algorithm for rendering
  **GRAPHICS_OPTIMIZATION_ENABLED:** Enable various optimizations for improved performance

## Architecture
The library is built around these core components:

* **Renderer:** Manages the rendering pipeline
* **Scene:** Holds the hierarchy of objects to render
* **Node:** Base class for scene elements
* **Component:** Base class for node functionality
* **Mesh:** Represents 3D geometry
* **Camera:** Controls the view and projection
* **FrameBuffer:** Manages the render target
* **Rasterizer:** Converts 3D geometry to pixels
* **DriverLCD:** Interface for display hardware

## Performance Considerations
The library is optimized for ESP32-P4 but can be configured for different performance profiles:

* Enable parallel processing on multi-core systems
* Use the appropriate display driver for your hardware
* Configure memory usage based on your application's requirements

## Contributing
Contributions are welcome! Please feel free to submit a Pull Request.

## License
This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments
Uses GLM (OpenGL Mathematics) library under the Happy Bunny License/MIT License
