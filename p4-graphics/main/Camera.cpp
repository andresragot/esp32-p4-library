//
//  Camera.cpp
//  FileToOBJ
//
//  Created by Andrés Ragot on 11/2/25.
//

#include "Camera.hpp"
#include <cmath>       /* sqrt */

namespace Ragot
{
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
    
    void Camera::normalize_direction()
    {
        /*float length = std::sqrt(transform.dir_x * transform.dir_x + transform.dir_y * transform.dir_y + transform.dir_z * transform.dir_z);
        if (length != 0.0f)
        {
            transform.dir_x /= length;
            transform.dir_y /= length;
            transform.dir_z /= length;
        }*/
    }
}
    
