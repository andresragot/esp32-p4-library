#include "ExtrudeMesh.hpp"
#include <iostream>

namespace Ragot
{
    using glm::fvec4;

    const char* ExtrudeMesh::EXTRUDE_TAG = "ExtrudeMesh";
    
    void ExtrudeMesh::log_mesh_info() const
    {
        ESP_LOGI(EXTRUDE_TAG, "=== ExtrudeMesh Info ===");
        
        glm::vec3 position = get_position();
        glm::vec3 rotation = get_rotation();
        glm::vec3 scale = get_scale();
        
        ESP_LOGI(EXTRUDE_TAG, "Posición: (%.2f, %.2f, %.2f)", 
                 position.x, position.y, position.z);
        ESP_LOGI(EXTRUDE_TAG, "Rotación: (%.2f, %.2f, %.2f)", 
                 rotation.x, rotation.y, rotation.z);
        ESP_LOGI(EXTRUDE_TAG, "Escala: (%.2f, %.2f, %.2f)", 
                 scale.x, scale.y, scale.z);
        
        ESP_LOGI(EXTRUDE_TAG, "Vértices: %zu", get_total_vertices());
        ESP_LOGI(EXTRUDE_TAG, "Caras: %zu", get_faces().size());
        
        // Mostrar algunos vértices para depuración
        const auto& vertices = get_vertices();
        if (vertices.size() > 0)
        {
            ESP_LOGI(EXTRUDE_TAG, "Muestra de vértices:");
            
            // Mostrar primer vértice
            ESP_LOGI(EXTRUDE_TAG, "  V[0] = (%.2f, %.2f, %.2f, %.2f)", 
                     vertices[0][0], vertices[0][1], vertices[0][2], vertices[0][3]);
            
            // Mostrar último vértice
            if (vertices.size() > 1)
            {
                ESP_LOGI(EXTRUDE_TAG, "  V[último] = (%.2f, %.2f, %.2f, %.2f)", 
                         vertices[vertices.size()-1][0], vertices[vertices.size()-1][1], 
                         vertices[vertices.size()-1][2], vertices[vertices.size()-1][3]);
            }
        }
    }

    void ExtrudeMesh::generate_faces ()
    {
        faces.clear();

        const auto &coords = mesh_info.coordinates;
        int total = int(coords.size());
        if (total % 2 != 0)
        {
            std::cerr << "Error: need even number of coords for bottom and top loops" << std::endl;
            return;
        }
        int n = total / 2;

        camPos = cam.get_position();

        // Precompute View-Projection matrix once
        glm::mat4 VP = cam.get_projection_matrix() * cam.get_view_matrix();

        // Build and cull vértices por frustrum y profundidad
        vertices.clear();
        std::vector<int> indexMap(total, -1);
        for (int i = 0; i < total; ++i)
        {
            const auto &pt = coords[i];
            float z = (i < n) ? 0.0f : height;
            glm::fvec4 worldPt(pt.x, pt.y, z, 1.0f);

            glm::vec4 clip = VP * worldPt;
            if (clip.w <= 0.0f) continue;
            glm::vec3 ndc = glm::vec3(clip) / clip.w;
            if (ndc.x < -1.0f || ndc.x > 1.0f ||
                ndc.y < -1.0f || ndc.y > 1.0f ||
                ndc.z <  0.0f || ndc.z > 1.0f)
                continue;

            // Depth cull
            glm::vec3 forward = glm::normalize(cam.get_view_direction());
            glm::vec3 dir   = glm::vec3(worldPt) - camPos;
            float depth     = glm::dot(dir, forward);
            if (depth < cam.get_near_plane() || depth > cam.get_far_plane())
                continue;

            indexMap[i] = int(vertices.size());
            vertices.push_back(worldPt);
        }

        // función auxiliar para culling de back‐faces
        auto is_face_visible = [&](const std::vector<int>& idxs) {
            // construye normal con los primeros tres puntos (el orden CCW importa)
            glm::vec3 p0 = glm::vec3(vertices[idxs[0]]);
            glm::vec3 p1 = glm::vec3(vertices[idxs[1]]);
            glm::vec3 p2 = glm::vec3(vertices[idxs[2]]);
            glm::vec3 normal = glm::normalize(glm::cross(p1 - p0, p2 - p0));
            glm::vec3 centroid(0.0f);
            for (int id : idxs)
                centroid += glm::vec3(vertices[id]);
            centroid /= float(idxs.size());

            glm::vec3 viewVec = glm::normalize(camPos - centroid);
            // sólo si la normal apunta hacia la cámara
            return glm::dot(normal, viewVec) > 0.0f;
        };

        // Side quads (CCW “outside”)
        for (int i = 0; i < n; ++i)
        {
            int j    = (i + 1) % n;
            int b0   = indexMap[i],    b1 = indexMap[j];
            int t0   = indexMap[i+n],  t1 = indexMap[j+n];
            if (b0<0||b1<0||t0<0||t1<0) continue;

            std::vector<int> idxs = { b0, b1, t1, t0 };
            if (!is_face_visible(idxs)) continue;

            faces.emplace_back(face_t{ true, b0, t0, t1, b1 });
        }

        // Caps
        if (faces_can_be_quads)
        {
            // bottom quad (normal down)
            {
                std::vector<int> idxs = { indexMap[0], indexMap[3], indexMap[2], indexMap[1] };
                if (std::all_of(idxs.begin(), idxs.end(), [&](int v){ return v>=0; }) &&
                    is_face_visible(idxs))
                {
                    faces.emplace_back(face_t{ true, idxs[0], idxs[1], idxs[2], idxs[3] });
                }
            }
            // top quad (normal up)
            {
                std::vector<int> idxs = { indexMap[n], indexMap[n+3], indexMap[n+2], indexMap[n+1] };
                if (std::all_of(idxs.begin(), idxs.end(), [&](int v){ return v>=0; }) &&
                    is_face_visible(idxs))
                {
                    faces.emplace_back(face_t{ true, idxs[0], idxs[1], idxs[2], idxs[3] });
                }
            }
        }
        else
        {
            // bottom fan triangles (CCW down)
            int center = indexMap[0];
            for (int i = 1; i + 1 < n; ++i)
            {
                int v1 = indexMap [ i ], v2 = indexMap [ i + 1 ];
                std::vector<int> idxs = { center, v1, v2 };
                if (center<0||v1<0||v2<0) continue;
                if (!is_face_visible(idxs)) continue;
                faces.emplace_back(face_t{ false, center, v1, v2, 0 });
            }
            // top fan triangles (CCW up)
            center = indexMap[n];
            for (int i = 1; i + 1 < n; ++i)
            {
                int v1 = indexMap [ n + i ], v2 = indexMap [ n + i + 1];
                std::vector<int> idxs = { center, v1, v2 };
                if (center < 0 || v1 < 0 || v2 < 0) continue;
                if (!is_face_visible(idxs)) continue;
                faces.emplace_back(face_t{ false, center, v2, v1, 0 });
            }
        }
    }
}

