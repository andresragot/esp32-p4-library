/**
 * @file ExtrudeMesh.hpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief This file implements the ExtrudeMesh class, which manages the extrusion of a mesh in 3D space.
 * The ExtrudeMesh class inherits from the Mesh class and provides methods to generate vertices and faces for the extruded mesh.
 * It also includes methods for culling faces based on the camera's view direction and logging mesh information.
 * @details The ExtrudeMesh class is designed to create a 3D mesh by extruding a 2D shape along a specified height.
 * It uses the GLM library for vector and matrix operations, and includes functionality for face culling based on the camera's view direction.
 * The class also provides a method to log detailed information about the mesh, including its position, rotation, scale, and vertex data.
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
#include "Camera.hpp"
#include <iostream>

namespace Ragot
{
    /**
     * @class ExtrudeMesh
     * @brief Represents a 3D mesh created by extruding a 2D shape along a specified height.
     * This class inherits from the Mesh class and provides methods to generate vertices and faces for the extruded mesh.
     * It also includes methods for culling faces based on the camera's view direction and logging mesh information.
     * @details The ExtrudeMesh class is designed to create a 3D mesh by extruding a 2D shape along a specified height.
     * It uses the GLM library for vector and matrix operations, and includes functionality for face culling based on the camera's view direction.
     * The class also provides a method to log detailed information about the mesh, including its position, rotation, scale, and vertex data.
     */
    class ExtrudeMesh : public Mesh
    {
    protected:
        /**
         * @brief Tag for logging messages related to the ExtrudeMesh class.
         */
        static const char* EXTRUDE_TAG;

        /**
         * @brief Height of the extrusion.
         * 
         * This value determines how far the 2D shape is extruded in the Z direction.
         */
        float height = 1.0f;

        /**
         * @brief Flag indicating whether the faces can be quads.
         * 
         * This flag is set to true if the number of vertices is a multiple of 8 or if it is exactly 4.
         * It determines how faces are generated in the mesh.
         */
        bool faces_can_be_quads = false;
        
        /**
         * @brief Camera reference for culling faces based on the camera's view direction.
         * 
         * This reference is used to determine which faces of the mesh are visible from the camera's perspective.
         */
        const Camera & cam;

        /**
         * @brief Array of planes used for culling faces.
         * 
         * This array contains the planes that define the view frustum of the camera.
         * It is used to determine which faces are visible and which can be culled.
         */
        glm::vec4 planes[4];

        /**
         * @brief Position of the camera in world space.
         * 
         * This vector represents the position of the camera in the 3D world.
         * It is used to calculate the visibility of faces based on the camera's position.
         */
        glm::vec3 camPos;
    
    public:
        /**
         * @brief Constructs an ExtrudeMesh object with the specified mesh information and camera reference.
         * 
         * @param mesh_info Information about the mesh to be extruded.
         * @param cam Reference to the camera used for culling faces.
         */
        ExtrudeMesh (mesh_info_t & mesh_info, const Camera & cam) : Mesh (mesh_info), cam (cam)
        {
            vertices.reserve (mesh_info.coordinates.size() * 2    );
               faces.reserve (mesh_info.coordinates.size() * 3 - 3);
            // Si son 14 vertices -> 39 - 28 = 11
            // Si son 12 vertices -> 33 - 24 = 9
            // Si son 10 vertices -> 27 - 20 = 7
            // Si son  8 vertices ->  9 - 16 = -7
            // Si son  6 vertices -> 15 - 12 = 3
            // Si son 4 vertices ->   9 -  5 = 4
            
            // % 8 porque como están las coordenadas duplicadas...
            faces_can_be_quads = (mesh_info.vertex_amount % 8 == 0 || mesh_info.vertex_amount == 4);
            generate_vertices();
            generate_faces();
            
            std::cout << "Etrude Vertices: " << vertices.size() << std::endl;
            std::cout << "Extrude Faces: " << faces.size() << std::endl; 
        }
        
        /**
         * @brief Default destructor for the ExtrudeMesh class.
         * 
         * This destructor is used to clean up resources when the ExtrudeMesh object is destroyed.
         */
       ~ExtrudeMesh() = default;
        
        /**
         * @brief Deleted copy constructor for the ExtrudeMesh class.
         * 
         * This constructor is deleted to prevent copying of ExtrudeMesh objects.
         */
        void generate_vertices () override;

        /**
         * @brief Generates the faces of the extruded mesh.
         * 
         * This method creates the faces of the mesh based on the vertices generated by the generate_vertices method.
         * It checks if the vertices are coplanar and creates either quads or triangles accordingly.
         */
        void generate_faces    () override;
        
        /**
         * @brief Verifies if four vertices are coplanar.
         * 
         * This method checks if the four vertices v1, v2, v3, and v4 are coplanar within a specified tolerance.
         * It uses the scalar triple product to determine coplanarity.
         * @param v1 First vertex in homogeneous coordinates.
         * @param v2 Second vertex in homogeneous coordinates.
         * @param v3 Third vertex in homogeneous coordinates.
         * @param v4 Fourth vertex in homogeneous coordinates.
         * @param tolerance Tolerance value for coplanarity check (default is 0.1).
         * @return true if the vertices are coplanar, false otherwise.
         * @details The method calculates the scalar triple product of the vectors formed by the vertices and checks if it is close to zero within the specified tolerance.
         * The scalar triple product is computed as the dot product of the first vector with the cross product of the other two vectors.
         * This method is useful for determining if a set of vertices can form a valid face in the mesh.
         * @note This method assumes that the vertices are provided in homogeneous coordinates (4D vectors).
         */
        bool are_vertices_coplanar (const glm::fvec4 & v1, const glm::fvec4 & v2, const glm::fvec4 & v3, const glm::fvec4 & v4, float tolerance = 0.1f);

        /**
         * @brief Logs detailed information about the mesh.
         */
        void log_mesh_info() const;
    };
}
