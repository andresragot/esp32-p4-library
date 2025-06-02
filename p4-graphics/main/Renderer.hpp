/**
 * @file Renderer.hpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief Implementation of the Renderer class for rendering scenes in the Ragot engine.
 * @details The Renderer class is responsible for rendering 3D scenes using a rasterization approach.
 * @version 1.0
 * @date 2025-06-01
 * 
 * @copyright Copyright (c) 2025
 * MIT License
 * 
 * Copyright (c) 2025 Andrés Ragot 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "Mesh.hpp"
#include "FrameBuffer.hpp"
#include "Rasterizer.hpp"
#include "Node.hpp"
#include "Scene.hpp"
#include <memory>
#include <string>

#if ESP_PLATFORM == 1
#ifdef CONFIG_IDF_TARGET_ESP32P4
#include "driver_ek79007.hpp"
#elif CONFIG_IDF_TARGET_ESP32S3
#include "Driver_ST7789.hpp"
#endif
#else
#include "Shader_Program.hpp"
#endif

namespace Ragot
{
    /**
     * @class Renderer
     * @brief Class for rendering scenes in the Ragot engine.
     * 
     * This class is responsible for rendering 3D scenes using a rasterization approach.
     * It manages the frame buffer, rasterizer, and scene to be rendered.
     */
    class Renderer
    {
    private:
        
        float accumulated_time = 0.f; ///< Accumulated time for rendering frames, used for timing and performance measurement.
        size_t iterations = 0; ///< Number of iterations for rendering, used for performance testing and optimization.
        static constexpr size_t number_of_iterations = 10000000000000000; /// Number of iterations for performance testing, can be adjusted for different scenarios.
        
        #if ESP_PLATFORM == 1
        #ifdef CONFIG_IDF_TARGET_ESP32P4
        DriverEK79007 driver; ///< Driver for the EK79007 display controller, used for rendering on ESP32-P4 devices.
        #elif CONFIG_IDF_TARGET_ESP32S3
        Driver_ST7789 driver; ///< Driver for the ST7789 display controller, used for rendering on ESP32-S3 devices.
        #endif
        #else
        std::unique_ptr < Shader_Program > quadShader       = nullptr; ///< Shader program for rendering a full-screen quad, used in non-ESP platforms.
        GLuint           quadVAO         = 0; ///< Vertex Array Object for the full-screen quad, used in non-ESP platforms.
        GLuint           quadVBO         = 0; ///< Vertex Buffer Object for the full-screen quad, used in non-ESP platforms.
        GLuint           quadEBO         = 0; ///< Element Buffer Object for the full-screen quad, used in non-ESP platforms.
        
        void initFullScreenQuad();  ///< Initializes the full-screen quad for rendering, used in non-ESP platforms.
        
        static const std::string   vertex_shader_code; ///< Vertex shader code for rendering, used in non-ESP platforms.
        static const std::string fragment_shader_code; ///< Fragment shader code for rendering, used in non-ESP platforms.
        #endif
        FrameBuffer < RGB565 > frame_buffer; ///< Frame buffer for rendering, used to store pixel data for the rendered scene.
        Scene * current_scene = nullptr; ///< Pointer to the current scene being rendered, allows access to scene data and objects.
        Rasterizer < RGB565 > rasterizer; ///< Rasterizer for rendering polygons in the frame buffer, responsible for filling polygons with color and handling depth testing.
        
        unsigned  width; ///< Width of the rendering area in pixels, used to define the size of the frame buffer and viewport.
        unsigned height; ///< Height of the rendering area in pixels, used to define the size of the frame buffer and viewport.
        
        bool initialized = false; ///< Flag to indicate if the renderer has been initialized, used to prevent re-initialization and ensure resources are set up correctly.

        std::atomic<bool> running = false; ///< Flag to indicate if the renderer is currently running, used to control rendering tasks and stop them gracefully.
        
    public:
        /**
         * @brief Construct a new Renderer object (Deleted).
         * 
         * This constructor is deleted to prevent default construction of the Renderer class.
         */
        Renderer () = delete;
        
        /**
         * @brief Constructs a Renderer with the specified width and height.
         * 
         * Initializes the renderer with the given dimensions and prepares the frame buffer and rasterizer.
         * 
         * @param width The width of the rendering area in pixels.
         * @param height The height of the rendering area in pixels.
         */
        Renderer (unsigned width, unsigned height);

        /**
         * @brief Default destructor for the Renderer class.
         * 
         * Cleans up resources used by the renderer.
         */
       ~Renderer () = default;

        std::vector < glm::fvec4 > transformed_vertices; ///< Vector to store transformed vertices for rendering, used to hold the vertices after applying transformations such as model, view, and projection matrices.
        std::vector < glm::ivec4 > display_vertices; ///< Vector to store display vertices for rendering, used to hold the vertices after applying viewport transformations and clipping.
       
        /**
         * @brief Sets the current scene to be rendered.
         * 
         * This method sets the current scene to be rendered by the renderer.
         * It allows the renderer to access the scene's objects and properties for rendering.
         * 
         * @param scene Pointer to the Scene object to be set as the current scene.
         */
        void set_scene (Scene * scene) { current_scene = scene; }
        
        /**
         * @brief Gets the current scene being rendered.
         * 
         * This method returns a pointer to the current scene being rendered by the renderer.
         * It allows access to the scene's objects and properties for rendering.
         * 
         * @return Scene* Pointer to the current Scene object.
         */
        void init ();

        /**
         * @brief Renders the current scene.
         * 
         * This method performs the rendering of the current scene by preparing matrices, transforming vertices,
         * and filling polygons in the frame buffer using the rasterizer.
         * It also handles depth testing and color filling for polygons.
         */
        void render ();

        /**
         * @brief Performs the rendering task in a separate thread.
         * 
         * This method runs the rendering task in a separate thread, allowing for asynchronous rendering.
         * 
         * @param stop_token 
         */
        void task_render (std::stop_token stop_token);

        /**
         * @brief Checks if the face defined by the indices is front-facing.
         * 
         * This method checks if the face defined by the indices is front-facing based on the projected vertices.
         * It uses the area of the face to determine its orientation.
         * 
         * @param projected_vertices Pointer to an array of projected vertices in clip space.
         * @param indices Pointer to the face structure containing vertex indices.
         * @return true if the face is front-facing, false otherwise.
         */
        bool is_frontface (const glm::fvec4 * const projected_vertices, const face_t * const indices);

        /**
         * @brief Starts the rendering process.
         * 
         * This method sets the running flag to true, indicating that the renderer is ready to start rendering.
         */
        void start() { running = true;  }

        /**
         * @brief Stops the rendering process.
         * 
         * This method sets the running flag to false, indicating that the renderer should stop rendering.
         */
        void  stop() { running = false; }
    };
}


