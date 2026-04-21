/**
 * @file CommonTypes.hpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief This file defines common types and structures used in the Ragot engine, including camera transformations, mesh information, and rendering flags.
 * @details The CommonTypes.hpp file provides essential data structures for representing camera transformations, vertex and face definitions, rendering flags, and mesh information.
 * It includes structures for camera position and direction, vertex coordinates, face definitions (triangles and quads), and rendering flags for different mesh types. The file also defines a constant for the mathematical constant PI.
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
#include <vector>
#include <cstdint>

namespace Ragot
{
    /**
     * @struct Camera_Transform
     * @brief Represents the transformation of a camera in 3D space.
     * 
     * This structure holds the position and direction of the camera, allowing for transformations in a 3D environment.
     */
    struct Camera_Transform
    {
        float x, y, z;            // Posición de la cámara
        float dir_x, dir_y, dir_z; // Dirección de la cámara (vector normalizado)
    };
    
    /**
     * @struct transform_t
     * @brief Represents a transformation in 3D space.
     * 
     * This structure holds the position, rotation (in Euler angles), and scale of an object in 3D space.
     */
    struct transform_t
    {
        float x, y, z;
        float alpha, beta, gamma;
        float scale;
    };
    
    /**
     * @struct vertex_t
     * @brief Represents a vertex in 3D space.
     * 
     * This structure holds the x, y, and z coordinates of a vertex.
     */
    struct vertex_t
    {
        float x;
        float y;
        float z;
    };
    
    /**
     * @struct face_t
     * @brief Represents a face in a 3D mesh.
     * 
     * This structure can represent either a triangle or a quadrilateral face, depending on the `is_quad` flag.
     */
    struct face_t
    {
        bool is_quad;
        int v1;
        int v2;
        int v3;
        int v4;
    };
    

    /**
     * @enum render_flag_t
     * @brief Flags for rendering types.
     * 
     * This enumeration defines different rendering flags that can be used to specify how a mesh should be rendered.
     */
    enum render_flag_t : uint8_t
    {
        RENDER_NONE,
        RENDER_REVOLUTION,
        RENDER_EXTRUDE,
        RENDER_MAX
    };
    

    /**
     * @struct coordinates_t
     * @brief Represents 2D coordinates.
     * 
     * This structure holds the x and y coordinates of a point in 2D space, typically used for mesh vertices.
     */
    struct coordinates_t
    {
        float x;
        float y;
    };
    

    /**
     * @struct mesh_info_t
     * @brief Represents information about a mesh.
     * 
     * This structure holds the number of vertices, rendering flags, and a vector of coordinates for a mesh.
     */
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
    
    /**
     * @brief Mathematical constant PI.
     * 
     * This constant represents the value of π (pi), which is approximately 3.141592653.
     */
    constexpr float PI = 3.141592653f;
}
