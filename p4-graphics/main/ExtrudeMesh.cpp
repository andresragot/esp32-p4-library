//  ExtrudeMesh.cpp
//  FileToOBJ
//
//  Created by Andrés Ragot on 10/3/25.
//

#include "ExtrudeMesh.hpp"
#include <iostream>
#include "Logger.hpp"

namespace Ragot
{
    using glm::fvec4;

    const char* ExtrudeMesh::EXTRUDE_TAG = "ExtrudeMesh";
    
    void ExtrudeMesh::log_mesh_info() const
    {
        logger.Log (EXTRUDE_TAG, 3, "=== ExtrudeMesh Info ===");
        
        glm::vec3 position = get_position();
        glm::vec3 rotation = get_rotation();
        glm::vec3 scale = get_scale();
        
        logger.Log (EXTRUDE_TAG, 3, "Posición: (%.2f, %.2f, %.2f)", 
                    position.x, position.y, position.z);
        logger.Log (EXTRUDE_TAG, 3, "Rotación: (%.2f, %.2f, %.2f)", 
                    rotation.x, rotation.y, rotation.z);
        logger.Log (EXTRUDE_TAG, 3, "Escala: (%.2f, %.2f, %.2f)",
                    scale.x, scale.y, scale.z);
        
        logger.Log (EXTRUDE_TAG, 3, "Vértices: %zu", get_total_vertices());
        logger.Log (EXTRUDE_TAG, 3, "Caras: %zu", get_faces().size());
        
        // Mostrar algunos vértices para depuración
        const auto& vertices = get_vertices();
        if (vertices.size() > 0)
        {
            logger.Log (EXTRUDE_TAG, 3, "Muestra de vértices:");
            
            // Mostrar primer vértice
            logger.Log (EXTRUDE_TAG, 3, "  V[0] = (%.2f, %.2f, %.2f, %.2f)", 
                        vertices[0][0], vertices[0][1], vertices[0][2], vertices[0][3]);
            
            // Mostrar último vértice
            if (vertices.size() > 1)
            {
                logger.Log (EXTRUDE_TAG, 3, "  V[último] = (%.2f, %.2f, %.2f, %.2f)", 
                            vertices[vertices.size()-1][0], vertices[vertices.size()-1][1], 
                            vertices[vertices.size()-1][2], vertices[vertices.size()-1][3]);
            }
        }
    }

#ifdef CONFIG_GRAPHICS_OPTIMIZATION_ENABLED
    void ExtrudeMesh::generate_faces ()
    {
        // 1) Prepara espacio local
        glm::mat4 worldToLocal   = glm::inverse(get_transform_matrix());
        glm::vec3 camDirLocal3   = glm::normalize(glm::vec3(worldToLocal * glm::vec4(
                                             cam.get_view_direction(), 0.0f)));
        glm::vec2 view2D         = glm::normalize(glm::vec2(camDirLocal3.x, camDirLocal3.y));

        const auto& coords = mesh_info.coordinates;
        int n = int(coords.size());
        std::vector<int> bottomIndex(n, -1), topIndex(n, -1);

        auto addVertex = [&](int idx, float z, std::vector<int>& mapLayer)
        {
            if (mapLayer[idx] < 0)
            {
                glm::fvec4 v{ coords[idx].x, coords[idx].y, z, 1.0f };
                mapLayer[idx] = int(vertices.size());
                vertices.push_back(v);
            }
            return mapLayer[idx];
        };

        // 2) Recorre cada arista y hace culling 2D
        for(int i = 0; i < n; ++i)
        {
            int j = (i+1) % n;
            glm::vec2 pi{ coords[i].x, coords[i].y },
                      pj{ coords[j].x, coords[j].y };
            glm::vec2 e        = pj - pi;
            glm::vec2 normal2D = glm::normalize(glm::vec2(e.y, -e.x));

            // descarta si no está de cara
            if (glm::dot(normal2D, view2D) <= -0.8f)
                continue;

            // añade vértices
            int i0 = addVertex(i,     0.0f, bottomIndex);
            int i1 = addVertex(j,     0.0f, bottomIndex);
            int i2 = addVertex(j, height,   topIndex);
            int i3 = addVertex(i, height,   topIndex);

            // genera QUAD con winding CCW “outward”
            faces.emplace_back(face_t{
                true,
                i0, // base i
                i1, // base j
                i2, // top  j
                i3  // top  i
            });
        }

        // --- CAPS (suelo y tapa) ---
        // 1) Calcula la normal del polígono base en XY
        glm::vec3 p0 = glm::vec3(coords[0].x, coords[0].y, 0.0f);
        glm::vec3 p1 = glm::vec3(coords[1].x, coords[1].y, 0.0f);
        glm::vec3 p2 = glm::vec3(coords[2].x, coords[2].y, 0.0f);
        glm::vec3 polyNormal = glm::normalize(glm::cross(p1 - p0, p2 - p0));
        
        // 2) Determina visibilidad respecto a la cámara en espacio local
        bool bottomVisible = glm::dot(polyNormal, camDirLocal3) < -.8f;
        bool topVisible    = glm::dot(polyNormal, camDirLocal3) > -.8f;

        if (faces_can_be_quads)
        {
            // BOTTOM QUAD (normal hacia abajo)
            if (bottomVisible)
            {
                // Asume al menos 4 vértices; si tu polígono no es rectangular, tendrás
                // que explotar a fan de tris o un poly‐ngon, pero aquí tomamos 4 límites
                int b0 = addVertex(0,       0.0f, bottomIndex);
                int b1 = addVertex(1,       0.0f, bottomIndex);
                int b2 = addVertex(n - 1,   0.0f, bottomIndex);
                int b3 = addVertex(n - 2,   0.0f, bottomIndex);
                faces.emplace_back(face_t{ true, b0, b1, b2, b3 });
            }

            // TOP QUAD (normal hacia arriba)
            if (topVisible)
            {
                int t0 = addVertex(0,     height, topIndex);
                int t1 = addVertex(1,     height, topIndex);
                int t2 = addVertex(n - 1, height, topIndex);
                int t3 = addVertex(n - 2, height, topIndex);
                faces.emplace_back(face_t{ true, t3, t0, t1, t2 });
            }
        }
        else
        {
            // BOTTOM FAN TRIANGLES
            if (bottomVisible)
            {
                int center = addVertex(0, 0.0f, bottomIndex);
                for (int i = 1; i + 1 < n; ++i)
                {
                    int v1 = addVertex(i,     0.0f, bottomIndex);
                    int v2 = addVertex(i + 1, 0.0f, bottomIndex);
                    faces.emplace_back(face_t{ false, center, v1, v2, 0 });
                }
            }
            // TOP FAN TRIANGLES
            if (topVisible)
            {
                int center = addVertex(0, height, topIndex);
                for (int i = 1; i + 1 < n; ++i)
                {
                    int v1 = addVertex(i,     height, topIndex);
                    int v2 = addVertex(i + 1, height, topIndex);
                    faces.emplace_back(face_t{ false, center, v2, v1, 0 });
                }
            }
        }

    }

    void ExtrudeMesh::generate_vertices () {}
#else
    void ExtrudeMesh::generate_vertices()
    {
        vertices.clear();
        float slice_height = height / 2;

        for (int s = 0; s < 2; ++s)
        {
            float z = s * slice_height;
            for (const auto& coord : mesh_info.coordinates)
            {
                fvec4 v (
                        coord.x,
                        coord.y,
                        z,
                        1.0f
                         );
                         
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
#endif

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
}
