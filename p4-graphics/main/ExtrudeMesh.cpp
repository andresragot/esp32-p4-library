//
//  ExtrudeMesh.cpp
//  FileToOBJ
//
//  Created by Andrés Ragot on 10/3/25.
//

#include "ExtrudeMesh.hpp"
#include <iostream>

namespace Ragot
{
    using glm::fvec4;

    bool ExtrudeMesh::are_vertices_coplanar (const fvec4 & v1,
        const fvec4 & v2, const fvec4 & v3, const fvec4 & v4, float tolerance)
    {
        float nx1 = (v2.y - v1.y) * (v3.z - v1.z) - (v2.z - v1.z) * (v3.y - v1.y);
        float ny1 = (v2.z - v1.z) * (v3.x - v1.x) - (v2.x - v1.x) * (v3.z - v1.z);
        float nz1 = (v2.x - v1.x) * (v3.y - v1.y) - (v2.y - v1.y) * (v3.x - v1.x);
        
        float d = - (nx1 * v1.x + ny1 * v1.y + nz1 * v1.z);
        float distance = (nx1 * v4.x + ny1 * v4.y + nz1 * v4.z + d) / sqrt(nx1 * nx1 + ny1 * ny1 + nz1 * nz1);
        
        return std::abs(distance) < tolerance;
    }

    void ExtrudeMesh::generate_vertices()
    {
        vertices.clear();
        float slice_height = height / 2;

        for (int s = 0; s < 2; ++s)
        {
            float z = s * slice_height;
            for (const auto& coord : mesh_info.coordinates)
            {
                fvec4 v;
                v.x = coord.x;
                v.y = coord.y;
                v.z = z;
                vertices.push_back(v);
            }
        }
    }
    
    void ExtrudeMesh::generate_faces()
    {
        faces.clear();
        
        int vertices_per_layer = int(mesh_info.coordinates.size());
        int top_offset = vertices_per_layer;

        // Generar caras laterales
        for (int i = 0; i < vertices_per_layer - 1; ++i)
        {
            const fvec4 & v1 = vertices [i];
            const fvec4 & v2 = vertices [i + 1];
            const fvec4 & v3 = vertices [top_offset + i];
            const fvec4 & v4 = vertices [top_offset + i + 1];
        
            if (are_vertices_coplanar(v1, v2, v3, v4))
            {
                face_t quad;
                quad.v1 = i;
                quad.v2 = i + 1;
                quad.v3 = top_offset + i + 1;
                quad.v4 = top_offset + i;
                quad.is_quad = true;
                faces.emplace_back(quad);
            }
            else
            {
                face_t tri1;
                tri1.v1 = i;
                tri1.v2 = i + 1;
                tri1.v3 = top_offset + i;
                tri1.v4 = 0;
                tri1.is_quad = false;
                faces.emplace_back(tri1);
                
                face_t tri2;
                tri2.v1 = i + 1;
                tri2.v2 = top_offset + i + 1;
                tri2.v3 = top_offset + i;
                tri2.v4 = 0;
                tri2.is_quad = false;
                faces.emplace_back(tri2);
            }
        }

        // Generar caras extremas (top y bottom)
        if (faces_can_be_quads)
        {
            // Bottom face como un único quad
            face_t bottom_quad;
            bottom_quad.v1 = 1;
            bottom_quad.v2 = 0;
            bottom_quad.v3 = 3;
            bottom_quad.v4 = 2;
            bottom_quad.is_quad = true;
            faces.emplace_back(bottom_quad);

            // Top face como un único quad
            face_t top_quad;
            top_quad.v1 = top_offset + 0;
            top_quad.v2 = top_offset + 1;
            top_quad.v3 = top_offset + 2;
            top_quad.v4 = top_offset + 3;
            top_quad.is_quad = true;
            faces.emplace_back(top_quad);
        }
        else
        {
            for (int i = 0; i < vertices_per_layer - 1; ++i)
            {
                face_t bottom_tri;
                bottom_tri.v1 = 0;
                bottom_tri.v2 = i + 1;
                bottom_tri.v3 = i;
                bottom_tri.v4 = 0;
                bottom_tri.is_quad = false;
                faces.emplace_back(bottom_tri);
            
                face_t top_tri;
                top_tri.v1 = top_offset;
                top_tri.v2 = top_offset + i;
                top_tri.v3 = top_offset + i + 1;
                top_tri.v4 = 0;
                top_tri.is_quad = false;
                faces.emplace_back(top_tri);
            }
        }
    }

    void ExtrudeMesh::generate_faces_direct(const Camera& cam) 
    {
        const auto& coords = mesh_info.coordinates;
        size_t n = coords.size();
        // Vector dirección de cámara (suponiendo que está normalizado)
        glm::fvec3 viewDir = cam.get_view_direction();
    
        // Para cada arista del perfil
        for (size_t i = 0; i < n; ++i) 
        {
            size_t j = (i + 1) % n;
    
            // Coordenadas 3D de los 4 vértices de la cara lateral
            glm::fvec3 b0{ coords[i].x,     coords[i].y,     0.0f };
            glm::fvec3 b1{ coords[j].x,     coords[j].y,     0.0f };
            glm::fvec3 t0{ coords[i].x,     coords[i].y,     height };
            glm::fvec3 t1{ coords[j].x,     coords[j].y,     height };
    
            // Normal “no normalizada” de todo el quad
            glm::fvec3 normal = glm::cross(b1 - b0, t0 - b0);
            // Culling: triángulo enfrente si dot(normal, viewDir) < 0
            if (glm::dot(normal, viewDir) < 0.0f) 
            {
                // Emitir dos triángulos (indices relativos:  
                // b0→i, b1→j, t0→i+n, t1→j+n)
                face_t tri1{ static_cast<int>(i),
                             static_cast<int>(j),
                             static_cast<int>(i + n),
                             0, false };
                face_t tri2{ static_cast<int>(j),
                             static_cast<int>(j + n),
                             static_cast<int>(i + n),
                             0, false };
                faces.push_back(tri1);
                faces.push_back(tri2);
            }
        }
    
        // --- tapas (puedes hacer un fan sin memoria extra) ---
        // Centro de la base
        glm::fvec3 baseC{ 0.0f, 0.0f, 0.0f };
        // Normal de tapa base: (0,0,-1), compare con viewDir directamente
        if (glm::dot(glm::fvec3{0,0,-1}, viewDir) < 0.0f) 
        {
            for (size_t i = 1; i + 1 < n; ++i) 
            {
                face_t f{  /*vCentro=*/static_cast<int>(2*n), 
                           /*v1=*/static_cast<int>(i),
                           /*v2=*/static_cast<int>(i+1),
                           0, false };
                faces.push_back(f);
            }
        }
        // Centro de la tapa superior
        glm::fvec3 topC{ 0.0f, 0.0f, height };
        if (glm::dot(glm::fvec3{0,0,1}, viewDir) < 0.0f) 
        {
            for (size_t i = 1; i + 1 < n; ++i) 
            {
                face_t f{  /*vCentro=*/static_cast<int>(2*n+1), 
                           /*v1=*/static_cast<int>(n + i + 1),
                           /*v2=*/static_cast<int>(n + i),
                           0, false };
                faces.push_back(f);
            }
        }
    }
    
}
