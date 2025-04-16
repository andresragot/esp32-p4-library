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

namespace Ragot
{
    class Renderer
    {
    private:
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


