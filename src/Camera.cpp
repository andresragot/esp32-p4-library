/**
 * @file Camera.cpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief This file implements the Camera class, which manages camera properties and operations in a 3D space.
 * @details The Camera class provides functionality to log camera information, calculate normals for faces, and determine visibility of faces based on their orientation relative to the camera.
 * It uses GLM (OpenGL Mathematics) for vector and matrix operations, and includes methods for calculating normals and checking face visibility.
 * @version 0.1
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

#define GLM_ENABLE_EXPERIMENTAL

#include "Camera.hpp"
#include <cmath>       /* sqrt */
#include <gtx/transform.hpp>   // para cross, normalize, etc.
#include <gtc/matrix_transform.hpp> // lookAt, perspective
#include <glm.hpp>                  // vec3, dot, normalize
#include "Logger.hpp"


namespace Ragot
{
    const char* Camera::CAMERA_TAG = "Camera";
    
    void Camera::log_camera_info() const
    {
        logger.Log (CAMERA_TAG, 3, "=== Camera Info ===");
        
        logger.Log (CAMERA_TAG, 3, "Posición: (%.2f, %.2f, %.2f)", 
                    position.x, position.y, position.z);	
        logger.Log (CAMERA_TAG, 3, "Target: (%.2f, %.2f, %.2f)", 
                    target.x, target.y, target.z);
        logger.Log (CAMERA_TAG, 3, "FOV: %.1f°", fov);
        logger.Log (CAMERA_TAG, 3, "Aspect Ratio: %.3f", aspect_ratio);
        logger.Log (CAMERA_TAG, 3, "Clipping: near=%.2f, far=%.2f", near_plane, far_plane);
        
        // Imprimir la dirección de vista (normalizada)
        glm::vec3 view_direction = glm::normalize(target - position);
        logger.Log (CAMERA_TAG, 3, "Dirección de vista: (%.2f, %.2f, %.2f)", 
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

