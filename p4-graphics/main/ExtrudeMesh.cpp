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

        ESP_LOGI(EXTRUDE_TAG, "Generando caras para extruir con %d coordenadas", total);
        camPos = cam.get_position();
        ESP_LOGI(EXTRUDE_TAG, "Cámara en posición (%.2f, %.2f, %.2f)", 
                 camPos.x, camPos.y, camPos.z);

        // Cálculo de culling de profundidad
        glm::vec3 forward = glm::normalize(cam.get_view_direction());
        float nearP = cam.get_near_plane();
        float farP  = cam.get_far_plane();

        // Precompute View-Projection matrix once
        glm::mat4 VP = cam.get_projection_matrix() * cam.get_view_matrix();

        // Build and cull vertices via NDC test in one pass
        vertices.clear();
        vertices.reserve(total);
        std::vector<int> indexMap(total, -1);
        for (int i = 0; i < total; ++i)
        {
            const auto &pt = coords[i];
            float z = (i < n) ? 0.0f : height;
            glm::fvec4 worldPt(pt.x, pt.y, z, 1.0f);
            // Project to clip space
            glm::vec4 clip = VP * worldPt;
            if (clip.w <= 0.0f) continue;
            glm::vec3 ndc = glm::vec3(clip) / clip.w;
            // Cull fuera de NDC
                if (ndc.x < -1.0f || ndc.x > 1.0f || ndc.y < -1.0f || ndc.y > 1.0f || ndc.z < 0.0f || ndc.z > 1.0f)
                    continue;
                // Culling de profundidad (near/far)
                glm::vec3 dir = glm::vec3(worldPt) - camPos;
                float depth = glm::dot(dir, forward);
                if (depth < nearP || depth > farP)
                    continue;
            indexMap[i] = int(vertices.size());
            vertices.push_back(worldPt);
        }

        // Después de procesar todos los vértices
        int accepted = 0;
        for (int i = 0; i < total; i++) {
            if (indexMap[i] >= 0) accepted++;
        }
        ESP_LOGI(EXTRUDE_TAG, "Vértices aceptados: %d de %d total", accepted, total);

        // Side quads (CCW outside)
        for (int i = 0; i < n; ++i)
        {
            int j = (i + 1) % n;
            int b0 = indexMap[i],   b1 = indexMap[j];
            int t0 = indexMap[i + n], t1 = indexMap[j + n];
            if (b0 < 0 || b1 < 0 || t0 < 0 || t1 < 0)
                continue;
            faces.emplace_back(face_t{true, b0, t0, t1, b1});
        }

        // Caps
        if (faces_can_be_quads)
        {
            int b0 = indexMap[0], b1 = indexMap[1], b2 = indexMap[2], b3 = indexMap[3];
            if (b0 >= 0 && b1 >= 0 && b2 >= 0 && b3 >= 0)
                // bottom quad (normal down)
                faces.emplace_back(face_t{true, b0, b3, b2, b1});

            int t0 = indexMap[n], t1 = indexMap[n + 1], t2 = indexMap[n + 2], t3 = indexMap[n + 3];
            if (t0 >= 0 && t1 >= 0 && t2 >= 0 && t3 >= 0)
                // top quad (normal up)
                faces.emplace_back(face_t{true, t0, t3, t2, t1});
        }
        else
        {
            // bottom fan triangles (CCW down)
            int center = indexMap[0];
            for (int i = 1; i + 1 < n; ++i)
            {
                int v1 = indexMap[i + 1], v2 = indexMap[i];
                if (center < 0 || v1 < 0 || v2 < 0)
                    continue;
                faces.emplace_back(face_t{false, center, v2, v1, 0});
            }
            // top fan triangles (CCW up)
            center = indexMap[n];
            for (int i = 1; i + 1 < n; ++i)
            {
                int v1 = indexMap[n + i], v2 = indexMap[n + i + 1];
                if (center < 0 || v1 < 0 || v2 < 0)
                    continue;
                faces.emplace_back(face_t{false, center, v2, v1, 0});
            }
        }

        // Antes de terminar generate_faces()
        ESP_LOGI(EXTRUDE_TAG, "Generadas %zu caras: %zu quads, %zu triángulos", 
                 faces.size(), 
                 std::count_if(faces.begin(), faces.end(), [](const face_t& f) { return f.is_quad; }),
                 std::count_if(faces.begin(), faces.end(), [](const face_t& f) { return !f.is_quad; }));
    }
}

