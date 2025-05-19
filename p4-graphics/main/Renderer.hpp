//
//  Renderer.hpp
//  FileToOBJ
//
//  Created by Andrés Ragot on 6/4/25.
//

#pragma once

#include "Mesh.hpp"
#include "FrameBuffer.hpp"
#include "Rasterizer.hpp"
#include "Node.hpp"
#include "Scene.hpp"
#include <memory>
#include <string>

#if ESP_PLATFORM == 1
#include "driver_ek79007.hpp"
#else
#include "Shader_Program.hpp"
#endif

namespace Ragot
{
    class Renderer
    {
    private:
        
        float accumulated_time = 0.f;
        size_t iterations = 0;
        static constexpr size_t number_of_iterations = 10000000000000000;
        
        #if ESP_PLATFORM == 1
        DriverEK79007 driver;
        #else
        std::unique_ptr < Shader_Program > quadShader       = nullptr;
        GLuint           quadVAO         = 0;
        GLuint           quadVBO         = 0;
        GLuint           quadEBO         = 0;
        
        void initFullScreenQuad();  // helper para crear VAO/VBO
        
        static const std::string   vertex_shader_code;
        static const std::string fragment_shader_code;
        #endif
        FrameBuffer < RGB565 > frame_buffer;
        Scene * current_scene = nullptr;
        Rasterizer < RGB565 > rasterizer;
        
        unsigned  width;
        unsigned height;
        
        bool initialized = false;
        
    public:
        Renderer () = delete;
        Renderer (unsigned width, unsigned height);
       ~Renderer () = default;

        std::vector < glm::fvec4 > transformed_vertices;
        std::vector < glm::ivec4 > display_vertices;
       
        void set_scene (Scene * scene) { current_scene = scene; }
        
        void init ();

        void render ();
        bool is_frontface (const glm::fvec4 * const projected_vertices, const face_t * const indices);
    };
}


