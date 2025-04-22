//
//  Camera.cpp
//  FileToOBJ
//
//  Created by Andrés Ragot on 11/2/25.
//
#define GLM_ENABLE_EXPERIMENTAL

#include "Camera.hpp"
#include <cmath>       /* sqrt */
#include <gtx/transform.hpp>   // para cross, normalize, etc.
#include <gtc/matrix_transform.hpp> // lookAt, perspective
#include <glm.hpp>                  // vec3, dot, normalize


namespace Ragot
{
    const char* Camera::CAMERA_TAG = "Camera";
    
    void Camera::log_camera_info() const
    {
        ESP_LOGI(CAMERA_TAG, "=== Camera Info ===");
        ESP_LOGI(CAMERA_TAG, "Posición: (%.2f, %.2f, %.2f)", 
                 position.x, position.y, position.z);
        ESP_LOGI(CAMERA_TAG, "Target: (%.2f, %.2f, %.2f)", 
                 target.x, target.y, target.z);
        ESP_LOGI(CAMERA_TAG, "FOV: %.1f°", fov);
        ESP_LOGI(CAMERA_TAG, "Aspect Ratio: %.3f", aspect_ratio);
        ESP_LOGI(CAMERA_TAG, "Clipping: near=%.2f, far=%.2f", near_plane, far_plane);
        
        // Imprimir la dirección de vista (normalizada)
        glm::vec3 view_direction = glm::normalize(target - position);
        ESP_LOGI(CAMERA_TAG, "Dirección de vista: (%.2f, %.2f, %.2f)", 
                 view_direction.x, view_direction.y, view_direction.z);
    }

    vertex_t Camera::calculate_normal(const vertex_t & v1, const vertex_t & v2, const vertex_t & v3)
    {
        vertex_t u = { v2.x - v1.x, v2.y - v1.y, v2.z - v1.z };
        vertex_t v = { v3.x - v1.x, v3.y - v1.y, v3.z - v1.z };
        
        vertex_t normal = {
            u.y * v.z - u.z * v.y,
            u.z * v.x - u.x * v.z,
            u.x * v.y - u.y * v.x
        };
        
        // Normalización con mejor precisión numérica
        float length = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
        const float EPSILON = 1e-6f;
        
        if (length > EPSILON)
        {
            float inv_length = 1.0f / length;
            normal.x *= inv_length;
            normal.y *= inv_length;
            normal.z *= inv_length;
        }
        
    return normal;
    }
    
    bool Camera::is_face_visible(const vertex_t & v1, const vertex_t & v2, const vertex_t & v3)
    {
        vertex_t normal = calculate_normal(v1, v2, v3);
        
        // Calcular el centro del triángulo
        /*vertex_t center = {
            (v1.x + v2.x + v3.x) * 0.333333333f,
            (v1.y + v2.y + v3.y) * 0.333333333f,
            (v1.z + v2.z + v3.z) * 0.333333333f
        };*/

        // Vector desde un punto de la cara hacia la cámara
        vertex_t to_camera = {
            0,
            0,
            0
        };
        

        // Normalizar to_camera
        float length = std::sqrt(to_camera.x * to_camera.x + to_camera.y * to_camera.y + to_camera.z * to_camera.z);
        const float EPSILON = 1e-6f;
        
        if (length > EPSILON)
        {
            float inv_length = 1.0f / length;
            to_camera.x *= inv_length;
            to_camera.y *= inv_length;
            to_camera.z *= inv_length;
        }

        // Producto escalar entre la normal y el vector hacia la cámara
        float dot = normal.x * to_camera.x + normal.y * to_camera.y + normal.z * to_camera.z;
        
        // Calcular el ángulo absoluto con el plano XZ (para caras superior/inferior)
        float y_angle = std::abs(normal.y);
        
        // Umbrales más estrictos
        const float VISIBILITY_THRESHOLD = 0.1f;     // Más restrictivo para visibilidad general
        const float VERTICAL_THRESHOLD = 2.f;       // Para detectar caras casi horizontales
        
        // Una cara es visible si:
        // 1. Está orientada hacia la cámara (dot producto positivo)
        // 2. No es una cara casi horizontal (y_angle bajo)
        return dot > VISIBILITY_THRESHOLD && y_angle < VERTICAL_THRESHOLD;

    }
}

