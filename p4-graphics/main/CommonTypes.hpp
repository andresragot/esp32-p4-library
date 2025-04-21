//
//  CommonTypes.hpp
//  FileToOBJ
//
//  Created by Andrés Ragot on 11/2/25.
//

#pragma once
#include <vector>
#include <cstdint>

namespace Ragot
{
    struct Camera_Transform
    {
        float x, y, z;            // Posición de la cámara
        float dir_x, dir_y, dir_z; // Dirección de la cámara (vector normalizado)
    };
    
    struct transform_t
    {
        float x, y, z;
        float alpha, beta, gamma;
        float scale;
    };
    
    struct vertex_t
    {
        float x;
        float y;
        float z;
    };
    
    struct face_t
    {
        bool is_quad;
        int v1;
        int v2;
        int v3;
        int v4;
    };
    
    enum render_flag_t : uint8_t
    {
        RENDER_NONE,
        RENDER_REVOLUTION,
        RENDER_EXTRUDE,
        RENDER_MAX
    };
    
    struct coordinates_t
    {
        float x;
        float y;
    };
    
    struct mesh_info_t
    {
        size_t vertex_amount = 0;
        render_flag_t render_flag = RENDER_NONE;
        std::vector < coordinates_t > coordinates;

        mesh_info_t () = default;
        mesh_info_t (std::vector < coordinates_t > & coords, render_flag_t flag)
        : vertex_amount (coords.size ()), render_flag (flag), coordinates (coords)
        {
        }
    };
    
    constexpr float PI = 3.141592653f;
}
