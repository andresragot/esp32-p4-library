//
//  Renderer.cpp
//  FileToOBJ
//
//  Created by Andrés Ragot on 6/4/25.
//

#include "Renderer.hpp"
#include "Camera.hpp"
#include <iostream>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>         // translate, rotate, scale, perspective
#include <gtc/type_ptr.hpp>                 // value_ptr, quat
#include "CommonTypes.hpp"

namespace Ragot
{
    using Matrix4x4 = glm::mat4;
    using glm::fvec4;

    Renderer::Renderer (unsigned width, unsigned height) : frame_buffer(width, height, true), rasterizer(frame_buffer), width(width), height(height)
    {
        
    }
    
    void Renderer::init()
    {
        if (current_scene && !initialized)
        {
            size_t total_vertices = 0;
            
            std::vector < Mesh * > meshes = current_scene->collect_components<Mesh>();
            
            for (auto mesh : meshes)
            {
                total_vertices += mesh->get_total_vertices();
            }
            
            transformed_vertices.reserve (total_vertices);
                display_vertices.reserve (total_vertices);
            
            initialized = true;
        }
    }
    
    void Renderer::render()
    {
        if (current_scene)
        {
            Camera * main_camera = current_scene->get_main_camera();
            
            Matrix4x4 view = main_camera->get_view_matrix();
            
            std::vector < Mesh * > meshes = current_scene->collect_components<Mesh>();
            
            Matrix4x4 projection = main_camera->get_projection_matrix();

            Matrix4x4 transformation;
                    
            for (auto mesh : meshes)
            {
                transformation = projection * view * mesh->get_transform_matrix();
                // projection * view * mesh;
                
                const std::vector < fvec4 > & vertices = mesh->get_vertices();
                
                for (size_t index = 0; index < mesh->get_total_vertices(); ++index)
                {
                    fvec4 & vertex = transformed_vertices[index] = transformation * vertices[index];
                    
                    float divisor = 1.f / vertex[3];
                    
                    vertex[0] *= divisor;
                    vertex[1] *= divisor;
                    vertex[2] *= divisor;
                    vertex[3]  = 1.f;
                }
            }
            
            Matrix4x4 identity(1);
            Matrix4x4 scaling = glm::scale (identity, glm::fvec3( float (width / 2), float (height / 2), 100000000.f ));
            Matrix4x4 translation = glm::translate (identity, glm::fvec3 (float (width / 2), float (height / 2), 0.f));
            transformation = translation * scaling;
            
            for (size_t index = 0, number_of_vertices = transformed_vertices.size (); index < number_of_vertices; ++index )
            {
                display_vertices [index] = glm::fvec4 (transformation * transformed_vertices [index]);
            }
            
            frame_buffer.clear_buffer();
            
            for (auto mesh : meshes)
            {
                const face_t * indices = mesh->get_faces().data();
                const face_t * end = indices + mesh->get_faces().size();
                for (; indices < end; )
                {
                    if (is_frontface(transformed_vertices.data(), indices))
                    {
                        rasterizer.set_color (RGB565(0xFF00FF));
                        
                        rasterizer.fill_convex_polygon_z_buffer (display_vertices.data(), indices);
                    }
                    
                    ++indices;
                }
            }
            
            frame_buffer.blit_to_window();
            frame_buffer.swap_buffers();
            
        }
        else
        {
            std::cerr << "No Camera associated" << std::endl;
        }
    }
    
    bool Renderer::is_frontface (const glm::fvec4 * const projected_vertices, const face_t * const indices )
    {
        bool response = false;
        if (indices->is_quad)
        {
            const glm::fvec4 & v0 = projected_vertices [indices->v1];
            const glm::fvec4 & v1 = projected_vertices [indices->v2];
            const glm::fvec4 & v2 = projected_vertices [indices->v3];
            const glm::fvec4 & v3 = projected_vertices [indices->v4];
            
            float area =   (v0 [0] * v1 [1] + v1 [0] * v2 [1] + v2 [0] * v3 [1] + v3 [0] * v0 [1])
                         - (v0 [1] * v1 [0] + v1 [1] * v2 [0] + v2 [1] * v3 [0] + v3 [1] * v0 [0]);
                         
            response = (area > 0.f);
        }
        else
        {
            const glm::fvec4 & v0 = projected_vertices [indices->v1];
            const glm::fvec4 & v1 = projected_vertices [indices->v2];
            const glm::fvec4 & v2 = projected_vertices [indices->v3];
            
            response = ((v1 [0] - v0 [0]) * (v2 [1] - v0 [1]) - (v2 [0] - v0 [0]) * (v1 [1] - v0 [1]) > 0.f);
        }
        
        return response;
    }
    
} // namespace Ragot
