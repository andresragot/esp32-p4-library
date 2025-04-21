//
//  Renderer.cpp
//  FileToOBJ
//
//  Created by Andrés Ragot on 6/4/25.
//

#include "Renderer.hpp"
#include "Camera.hpp"
#include <iostream>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>         // translate, rotate, scale, perspective
#include <gtc/type_ptr.hpp>                 // value_ptr, quat
#include "CommonTypes.hpp"
#include "esp_log.h"
#include "esp_heap_caps.h"

namespace Ragot
{
    using Matrix4x4 = glm::mat4;
    using glm::fvec4;
    static const char* RENDERER_TAG = "Renderer";

#ifdef DEBUG
    const int Renderer::vertices[][4] =
    {
        { -4, -4, +4, 1 },      // 0    // vértices del cubo
        { +4, -4, +4, 1 },      // 1
        { +4, +4, +4, 1 },      // 2
        { -4, +4, +4, 1 },      // 3
        { -4, -4, -4, 1 },      // 4
        { +4, -4, -4, 1 },      // 5
        { +4, +4, -4, 1 },      // 6
        { -4, +4, -4, 1 },      // 7
        { -5, -5,  0, 1 },      // 8    // vértices de los polígonos cortantes
        { +5, -5,  0, 1 },      // 9
        { +5, +5,  0, 1 },      // 10
        { -5, +5,  0, 1 },      // 11
        {  0, -5, +5, 1 },      // 12
        {  0, +5, +5, 1 },      // 13
        {  0, +5, -5, 1 },      // 14
        {  0, -5, -5, 1 },      // 15
    };
        
    const float Renderer::colors[][3] =
    {
        { 1,   0,   0 },      // 0
        { 0,   1,   0 },      // 1
        { 0,   0,   1 },      // 2
        { 0,   0,   1 },      // 3
        { 1,   1,   0 },      // 4
        { 1,   0,   1 },      // 5
        { 1,   0,   0 },      // 6
        { 1,   1,   0 },      // 7
        { 1, .7f, .7f },      // 8
        { 1, .7f, .7f },      // 9
        { 1, .7f, .7f },      // 10
        { 1, .7f, .7f },      // 11
        { 1,   1, .9f },      // 12
        { 1,   1, .9f },      // 13
        { 1,   1, .9f },      // 14
        { 1,   1, .9f },      // 15
    };
        
    const int Renderer::triangles[][3] =
    {
        {  0,  1,  2 },         // cube front
        {  0,  2,  3 },
        {  4,  0,  3 },         // cube left
        {  4,  3,  7 },
        {  5,  4,  7 },         // cube back
        {  5,  7,  6 },
        {  1,  5,  6 },         // cube right
        {  1,  6,  2 },
        {  3,  2,  6 },         // cube top
        {  3,  6,  7 },
        {  0,  4,  5 },         // cube bottom
        {  0,  5,  1 },
        {  8,  9, 10 },         // middle frontface
        {  8, 10, 11 },
        { 10,  9,  8 },         // middle backface
        { 11, 10,  8 },
        { 12, 13, 14 },         // middle leftface
        { 12, 14, 15 },
        { 14, 13, 12 },         // middle rightface
        { 15, 14, 12 },
    };
#endif
    Renderer::Renderer (unsigned width, unsigned height) 
    : driver(), frame_buffer(width, height, true), rasterizer(frame_buffer), width(width), height(height)
    {
#ifdef DEBUG
        size_t number_of_vertices = sizeof(vertices) / sizeof(int) / 4;

        original_vertices.resize (number_of_vertices);

        for (size_t index = 0; index < number_of_vertices; index++)
        {
            original_vertices[index] = fvec4 (vertices[index][0], vertices[index][1], vertices[index][2], vertices[index][3]);
        }

        transformed_vertices.resize (number_of_vertices);
            display_vertices.resize (number_of_vertices);

        // Se definen los datos de color de los vértices:

        size_t number_of_colors = sizeof(colors) / sizeof(int) / 3;

        assert(number_of_colors == number_of_vertices);             // Debe haber el mismo número
                                                                    // de colores que de vértices
        original_colors.resize (number_of_colors);

        for (size_t i = 0; i < number_of_colors; i++)
        {
                // Escalado y redondeo
            int r = static_cast<int>(std::round(colors[i][0] * 31.0f));
            int g = static_cast<int>(std::round(colors[i][1] * 63.0f));
            int b = static_cast<int>(std::round(colors[i][2] * 31.0f));

            // Clamp por si acaso
            r = std::clamp(r, 0, 31);
            g = std::clamp(g, 0, 63);
            b = std::clamp(b, 0, 31);

            // Empaquetar en 16 bits: RRRRRGGGGGGBBBBB
            original_colors[i] = static_cast<uint16_t>((r << 11) | (g << 5) | b);
        }

        // Se generan los índices de los triángulos:

        size_t number_of_triangles = sizeof(triangles) / sizeof(int) / 3;

        original_indices.resize (number_of_triangles * 3);

        std::vector < int >::iterator indices_iterator = original_indices.begin ();

        for (size_t triangle_index = 0; triangle_index < number_of_triangles; triangle_index++)
        {
            *indices_iterator++ = triangles[triangle_index][0];
            *indices_iterator++ = triangles[triangle_index][1];
            *indices_iterator++ = triangles[triangle_index][2];
        }
#else
        init();
#endif
    }
    
    void Renderer::init()
    {
        if (current_scene && !initialized)
        {
            size_t total_vertices = 0;
            
            std::vector < Mesh * > meshes = current_scene->collect_components<Mesh>();
            
            for (auto mesh : meshes)
            {
                total_vertices += mesh->get_total_vertices();
            }
            
            transformed_vertices.resize (total_vertices);
                display_vertices.resize (total_vertices);
            
            initialized = true;
        }
    }
    
    void Renderer::render()
    {
        ESP_LOGI(RENDERER_TAG, "==== Iniciando render() ====");
        ESP_LOGI(RENDERER_TAG, "Memoria libre: %u bytes", esp_get_free_heap_size());
        
        if (current_scene)
        {
            ESP_LOGI(RENDERER_TAG, "Escena actual encontrada, obteniendo cámara principal");
            Camera * main_camera = current_scene->get_main_camera();
            
            Matrix4x4 view = main_camera->get_view_matrix();
            ESP_LOGI(RENDERER_TAG, "Matriz de vista obtenida");
            
            std::vector < Mesh * > meshes = current_scene->collect_components<Mesh>();
            ESP_LOGI(RENDERER_TAG, "Encontrados %zu meshes en la escena", meshes.size());
            
            Matrix4x4 projection = main_camera->get_projection_matrix();
            ESP_LOGI(RENDERER_TAG, "Matriz de proyección obtenida (aspect ratio: %.3f)", float(height) / width);

            Matrix4x4 transformation;
                    
            int total_vertices_transformed = 0;
            for (auto mesh : meshes)
            {
                // ESP_LOGI(RENDERER_TAG, "Procesando mesh con %zu vértices", mesh->get_total_vertices());
                transformation = projection * view * mesh->get_transform_matrix();
                
                const std::vector < fvec4 > & vertices = mesh->get_vertices();
                
                for (size_t index = 0; index < mesh->get_total_vertices(); ++index)
                {
                    fvec4 & vertex = transformed_vertices[index] = transformation * vertices[index];
                    
                    float divisor = 1.f / vertex[3];
                    
                    vertex[0] *= divisor;
                    vertex[1] *= divisor;
                    vertex[2] *= divisor;
                    vertex[3]  = 1.f;
                    
                    // if (index == 0 || index == mesh->get_total_vertices() - 1) 
                    // {
                    //     ESP_LOGI(RENDERER_TAG, "Vértice %zu transformado: (%.2f, %.2f, %.2f, %.2f)",
                    //              index, vertex[0], vertex[1], vertex[2], vertex[3]);
                    // }
                }
                total_vertices_transformed += mesh->get_total_vertices();
            }
            ESP_LOGI(RENDERER_TAG, "Total de vértices transformados: %d", total_vertices_transformed);
            
            ESP_LOGI(RENDERER_TAG, "Transformando a coordenadas de pantalla (width=%u, height=%u)", width, height);
            Matrix4x4 identity(1);
            Matrix4x4 scaling = glm::scale (identity, glm::fvec3( float (height / 2), float (width / 2), 100000000.f ));
            Matrix4x4 translation = glm::translate (identity, glm::fvec3 (float (height / 2), float (width / 2), 0.f));
            transformation = translation * scaling;
            
            ESP_LOGI(RENDERER_TAG, "Aplicando transformación de pantalla a %zu vértices", transformed_vertices.size());
            for (size_t index = 0, number_of_vertices = transformed_vertices.size (); index < number_of_vertices; ++index )
            {
                display_vertices [index] = glm::fvec4 (transformation * transformed_vertices [index]);
                
                // if (index == 0 || index == number_of_vertices - 1) 
                // {
                //     ESP_LOGI(RENDERER_TAG, "Vértice %zu en pantalla: (%.2f, %.2f, %.2f, %.2f)",
                //              index, display_vertices[index][0], display_vertices[index][1], 
                //              display_vertices[index][2], display_vertices[index][3]);
                // }
            }
            
            ESP_LOGI(RENDERER_TAG, "Iniciando renderizado de triángulos");
            int frontfaces = 0, backfaces = 0;
            for (auto mesh : meshes)
            {
                const face_t * indices = mesh->get_faces().data();
                const face_t * end = indices + mesh->get_faces().size();
                // ESP_LOGI(RENDERER_TAG, "Procesando %zu caras", mesh->get_faces().size());
                
                for (; indices < end; )
                {
                    if (is_frontface(transformed_vertices.data(), indices))
                    {
                        frontfaces++;
                        rasterizer.set_color (RGB565(0xFFFFFF));
                        
                        // ESP_LOGI(RENDERER_TAG, "Rasterizando cara %d (frontface)", frontfaces);
                        rasterizer.fill_convex_polygon_z_buffer (display_vertices.data(), indices);
                    }
                    else {
                        backfaces++;
                    }
                    
                    ++indices;
                }
            }
            ESP_LOGI(RENDERER_TAG, "Total caras procesadas: %d frontfaces, %d backfaces", frontfaces, backfaces);

            ESP_LOGI(RENDERER_TAG, "Enviando framebuffer al driver");
            esp_err_t result = driver.send_frame_buffer(frame_buffer.get_buffer());
            
            if (result == ESP_OK) 
            {
                ESP_LOGI(RENDERER_TAG, "Framebuffer enviado correctamente");
            } 
            else 
            {
                ESP_LOGE(RENDERER_TAG, "Error al enviar framebuffer: %s", esp_err_to_name(result));
            }
            
            ESP_LOGI(RENDERER_TAG, "Swapping y limpiando buffers");
            frame_buffer.swap_buffers();
            frame_buffer.clear_buffer();
            
        }
        else
        {
            ESP_LOGW(RENDERER_TAG, "No hay escena actual o cámara principal");
            std::cerr << "No Camera associated" << std::endl;
        }
        
        ESP_LOGI(RENDERER_TAG, "==== Render completado ====\n");
    }
    
    bool Renderer::is_frontface (const glm::fvec4 * const projected_vertices, const face_t * const indices )
    {
        bool response = false;
        if (indices->is_quad)
        {
            const glm::fvec4 & v0 = projected_vertices [indices->v1];
            const glm::fvec4 & v1 = projected_vertices [indices->v2];
            const glm::fvec4 & v2 = projected_vertices [indices->v3];
            const glm::fvec4 & v3 = projected_vertices [indices->v4];
            
            float area =   (v0 [0] * v1 [1] + v1 [0] * v2 [1] + v2 [0] * v3 [1] + v3 [0] * v0 [1])
                         - (v0 [1] * v1 [0] + v1 [1] * v2 [0] + v2 [1] * v3 [0] + v3 [1] * v0 [0]);
                         
            response = (area > 0.f);
        }
        else
        {
            const glm::fvec4 & v0 = projected_vertices [indices->v1];
            const glm::fvec4 & v1 = projected_vertices [indices->v2];
            const glm::fvec4 & v2 = projected_vertices [indices->v3];
            
            response = ((v1 [0] - v0 [0]) * (v2 [1] - v0 [1]) - (v2 [0] - v0 [0]) * (v1 [1] - v0 [1]) > 0.f);
        }
        
        return response;
    }

#ifdef DEBUG
    void Renderer::render_debug()
    {
        ESP_LOGI(RENDERER_TAG, "==== Iniciando render_debug() ====");
        ESP_LOGI(RENDERER_TAG, "Memoria libre: %u bytes", esp_get_free_heap_size());
        
        // Se actualizan los parámetros de transformatión (sólo se modifica el ángulo):

        static float angle = 0.f;

        angle += 0.025f;
        ESP_LOGI(RENDERER_TAG, "Ángulo actualizado: %.3f", angle);

        // Se crean las matrices de transformación:
        ESP_LOGI(RENDERER_TAG, "Creando matrices de transformación");

        Matrix4x4 identity(1);
        Matrix4x4 scaling     = glm::scale           (identity, glm::fvec3(0.75f)); 
        Matrix4x4 rotation_x  = glm::rotate          (identity, -0.65f, glm::vec3(1.f, 0.f, 0.f));
        Matrix4x4 rotation_y  = glm::rotate          (identity, angle, glm::vec3(0.f, 1.f, 0.f));
        Matrix4x4 translation = glm::translate       (identity, glm::fvec3{ 0, 0, -10 });
        Matrix4x4 projection  = glm::perspective     (20.f, float(height) / width, 1.f, 15.f);

        // Creación de la matriz de transformación unificada:

        Matrix4x4 transformation = projection * translation * rotation_x * rotation_y * scaling;
        
        ESP_LOGI(RENDERER_TAG, "Transformando %zu vértices", original_vertices.size());

        // Se transforman todos los vértices usando la matriz de transformación resultante:

        for (size_t index = 0, number_of_vertices = original_vertices.size (); index < number_of_vertices; index++)
        {
            // Se multiplican todos los vértices originales con la matriz de transformación y
            // se guarda el resultado en otro vertex buffer:

            fvec4 & vertex = transformed_vertices[index] = transformation * original_vertices[index];

            // La matriz de proyección en perspectiva hace que el último componente del vector
            // transformado no tenga valor 1.0, por lo que hay que normalizarlo dividiendo:

            float divisor = 1.f / vertex[3];

            vertex[0] *= divisor;
            vertex[1] *= divisor;
            vertex[2] *= divisor;
            vertex[3]  = 1.f;
            
            if (index == 0 || index == number_of_vertices - 1) {
                ESP_LOGI(RENDERER_TAG, "Vértice %zu transformado: (%.2f, %.2f, %.2f, %.2f)",
                         index, vertex[0], vertex[1], vertex[2], vertex[3]);
            }
        }

        ESP_LOGI(RENDERER_TAG, "Transformación a coordenadas de pantalla");
        // Se convierten las coordenadas transformadas y proyectadas a coordenadas
        // de recorte (-1 a +1) en coordenadas de pantalla con el origen centrado.
        // La coordenada Z se escala a un valor suficientemente grande dentro del
        // rango de int (que es lo que espera fill_convex_polygon_z_buffer).

        identity = Matrix4x4(1);
        scaling        = glm::scale (identity, glm::fvec3 (float(height / 2), float(width / 2), 100000000.f));
        translation    = glm::translate (identity, glm::fvec3{ float(height / 2), float(width / 2), 0.f });
        transformation = translation * scaling;

        for (size_t index = 0, number_of_vertices = transformed_vertices.size (); index < number_of_vertices; index++)
        {
            display_vertices[index] = glm::ivec4( transformation * transformed_vertices[index] );
            
            if (index == 0 || index == number_of_vertices - 1) {
                ESP_LOGI(RENDERER_TAG, "Vértice %zu en pantalla: (%.2f, %.2f, %.2f, %.2f)",
                         index, display_vertices[index][0], display_vertices[index][1], 
                         display_vertices[index][2], display_vertices[index][3]);
            }
        }

        ESP_LOGI(RENDERER_TAG, "Limpiando rasterizador");
        // Se borra el framebúffer y se dibujan los triángulos:

        rasterizer.clear ();

        ESP_LOGI(RENDERER_TAG, "Dibujando %zu triángulos", original_indices.size() / 3);
        int frontfaces = 0;
        int backfaces = 0;

        for (int * indices = original_indices.data (), * end = indices + original_indices.size (); indices < end; indices += 3)
        {
            if (is_frontface (transformed_vertices.data (), indices))
            {
                frontfaces++;
                // Se establece el color del polígono a partir del color de su primer vértice:
                uint16_t color = original_colors[*indices];
                rasterizer.set_color(color);
                
                ESP_LOGI(RENDERER_TAG, "Triángulo frontface %d: índices [%d, %d, %d], color: 0x%04X",
                         frontfaces, indices[0], indices[1], indices[2], color);

                // Se rellena el polígono:
                rasterizer.fill_convex_polygon_z_buffer (display_vertices.data (), indices, indices + 3);
            }
            else 
            {
                backfaces++;
            }
        }
        ESP_LOGI(RENDERER_TAG, "Total triángulos procesados: %d frontfaces, %d backfaces", frontfaces, backfaces);

        ESP_LOGI(RENDERER_TAG, "Enviando framebuffer al driver...");
        // Verificar el buffer antes de enviarlo
        const void * buffer = frame_buffer.get_buffer();
        if (buffer == nullptr) 
        {
            ESP_LOGE(RENDERER_TAG, "¡ERROR! El framebuffer es NULL");
        } 
        else 
        {
            ESP_LOGI(RENDERER_TAG, "Buffer en dirección: %p, tamaño: %u bytes", 
                    buffer, width * height * sizeof(uint16_t));
        }
        
        // Se copia el framebúffer oculto en el framebúffer de la ventana:
        ESP_LOGI(RENDERER_TAG, "Llamando a driver.send_frame_buffer()");
        esp_err_t result = driver.send_frame_buffer(frame_buffer.get_buffer());
        
        if (result == ESP_OK) 
        {
            ESP_LOGI(RENDERER_TAG, "Framebuffer enviado correctamente");
        } 
        else 
        {
            ESP_LOGE(RENDERER_TAG, "Error al enviar framebuffer: %s", esp_err_to_name(result));
        }
        
        ESP_LOGI(RENDERER_TAG, "Llamando a frame_buffer.swap_buffers()");
        frame_buffer.swap_buffers();

        frame_buffer.clear_buffer();
        
        ESP_LOGI(RENDERER_TAG, "==== Render completado ====\n");
    }

    bool Renderer::is_frontface (const glm::fvec4 * const projected_vertices, const int * const indices)
    {
        const glm::fvec4 & v0 = projected_vertices[indices[0]];
        const glm::fvec4 & v1 = projected_vertices[indices[1]];
        const glm::fvec4 & v2 = projected_vertices[indices[2]];

        // Se asumen coordenadas proyectadas y polígonos definidos en sentido horario.
        // Se comprueba a qué lado de la línea que pasa por v0 y v1 queda el punto v2:

        return ((v1[0] - v0[0]) * (v2[1] - v0[1]) - (v2[0] - v0[0]) * (v1[1] - v0[1]) > 0.f);
    }
#endif
    
} // namespace Ragot
