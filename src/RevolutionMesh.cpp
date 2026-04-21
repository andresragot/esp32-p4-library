/**
 * @file RevolutionMesh.cpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief Implementation of the RevolutionMesh class for generating revolution meshes.
 * @details The RevolutionMesh class generates a mesh by revolving a 2D profile around an axis.
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

#include "RevolutionMesh.hpp"
#include <iostream>

namespace Ragot
{
    using namespace glm;
    
#ifdef CONFIG_GRAPHICS_OPTIMIZATION_ENABLED
    void RevolutionMesh::generate_faces()
    {
        vertices.clear();
        faces.clear();

        const auto& coords = mesh_info.coordinates;
        int C = int(coords.size());
        int S = slices;
        float inverseS = 1.0f / float(S);

        // 1) calculamos dirección de vista en espacio local
        mat4 worldToLocal = glm::inverse(get_transform_matrix());
        vec3 viewDirLocal = glm::normalize(vec3(
        worldToLocal * vec4(cam.get_view_direction(), 0.0f)));

        // 2) mapa de índices [slice][coord] → índice en vertices[]
        std::vector<std::vector<int>> indexMap(S+1, std::vector<int>(C, -1));

        auto addVertex = [&](int slice, int j)
        {
            int &dest = indexMap[slice][j];
            if (dest < 0)
            {
                float theta = (2.0f * PI * slice) * inverseS;
                float c = cos(theta), s = sin(theta);
                auto &pt = coords[j];
                // coordenadas en mesh local
                fvec4 v{ pt.x * c, pt.y, pt.x * s, 1.0f };
                dest = int(vertices.size());
                vertices.push_back(v);
            }
            return dest;
        };

        // 3) recorremos cada quad potencial
        for (int slice = 0; slice < S; ++slice)
        {
            int nextSlice = slice + 1;
            for (int j = 0; j < C; ++j)
            {
                int j2 = (j+1) % C;

                // obtener posiciones temporales para calcular normal
                float t0 = (2.0f*PI*slice)   / float(S),
                      t1 = (2.0f*PI*nextSlice)/ float(S);
                auto &p0c = coords[j];
                auto &p1c = coords[j2];

                vec3 p0{ p0c.x * cos(t0), p0c.y, p0c.x * sin(t0) };
                vec3 p1{ p0c.x * cos(t1), p0c.y, p0c.x * sin(t1) };
                vec3 p2{ p1c.x * cos(t1), p1c.y, p1c.x * sin(t1) };

                // normal del quad (p0,p1,p2)
                vec3 normal = glm::normalize(glm::cross(p1 - p0, p2 - p0));
                if (glm::dot(normal, viewDirLocal) <= -0.8f)
                    continue;  // no está de cara

                // si llegó acá, la cara se ve: añadimos vértices y el face
                int v0 = addVertex(slice,   j);
                int v1 = addVertex(slice,   j2);
                int v2 = addVertex(nextSlice, j2);
                int v3 = addVertex(nextSlice, j);

                faces.emplace_back(face_t{
                    /*is_quad=*/true,
                    /*v1=*/v0,
                    /*v2=*/v1,
                    /*v3=*/v2,
                    /*v4=*/v3
                });
            }
        }

        // --- capas (bottom & top) ---
        // normal del perfil en z=0
        vec3 a = vec3(coords[0].x, coords[0].y, 0.0f);
        vec3 b = vec3(coords[1].x, coords[1].y, 0.0f);
        vec3 c = vec3(coords[2].x, coords[2].y, 0.0f);
        vec3 polyNormal = glm::normalize(glm::cross(b - a, c - a));

        bool bottomVisible = glm::dot(polyNormal, viewDirLocal) < -0.8f;
        bool   topVisible = glm::dot(polyNormal, viewDirLocal) > -0.8f;

        if (bottomVisible)
        {
            // int center = addVertex(0, 0);
            for (int j = 1; j + 1 < C; ++j)
            {
                int v1 = addVertex(0, j);
                int v2 = addVertex(0, j+1);
                // faces.emplace_back(face_t{false, 0, v1, v2, 0});
            }
        }
        if (topVisible)
        {
            int center = addVertex(S, 0);
            for (int j = 1; j + 1 < C; ++j)
            {
                int v1 = addVertex(S, j);
                int v2 = addVertex(S, j+1);
                // CW para que apunte hacia arriba
                // faces.emplace_back(face_t{false, center, v2, v1, 0});
            }
        }
    }
    
    void RevolutionMesh::generate_vertices() {}
#else
    // Old way
    using glm::fvec4;
    void RevolutionMesh::generate_vertices()
    {
        vertices.clear();
        
        for (int i = 0; i <= slices; ++i)
        {
            float theta = (2.0f * PI * i) / slices;
            float cos_theta = cos (theta);
            float sin_theta = sin (theta);
            
            for (const auto & coord : mesh_info.coordinates)
            {
                fvec4 v (
                        coord.x * cos_theta,
                        coord.y,
                        coord.x * sin_theta,
                        1.0f
                         );
                
                
                vertices.push_back(v);
            }
        }
        
        // for (const auto& vert : vertices)
        {
            // std::cout << "Vertex: (" << vert.x << ", " << vert.y << ", " << vert.z << ")" << std::endl;
        }
    }
    
    void RevolutionMesh::generate_faces()
    {
        faces.clear();
        int coords_size = int(mesh_info.coordinates.size());
        
        for (int i = 0; i < slices; ++i)
        {
            int next_slice = (i + 1);
            for (int j = 0; j < coords_size; ++j)
            {
                int next_j = (j + 1) % coords_size;
                
                face_t face1;
                face1.v1 = next_slice * coords_size + j;
                face1.v2 = i * coords_size + j;
                face1.v3 = i * coords_size + next_j;
                face1.v4 = next_slice * coords_size + next_j;
                face1.is_quad = true;
                
                faces.emplace_back(face1);
            }
        }
    }
#endif
}
