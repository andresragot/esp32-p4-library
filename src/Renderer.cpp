/**
 * @file Renderer.cpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief Implementation of the Renderer class for rendering scenes in the Ragot engine.
 * @details The Renderer class is responsible for rendering 3D scenes using a rasterization approach.
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

#include "Renderer.hpp"
#include "Camera.hpp"
#include <iostream>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>         // translate, rotate, scale, perspective
#include <gtc/type_ptr.hpp>                 // value_ptr, quat
#include "CommonTypes.hpp"
#include "Logger.hpp"
#include <memory>
#include <chrono>
#include <thread>

#ifdef CONFIG_GRAPHICS_PARALLEL_ENABLED
#include "Thread_Pool.hpp"
#if defined(ESP_PLATFORM) && ESP_PLATFORM == 1
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif
#endif

namespace Ragot
{
    using Matrix4x4 = glm::mat4;
    using glm::fvec4;
    static const char* RENDERER_TAG = "Renderer";

    Renderer::Renderer (unsigned width, unsigned height, DriverLCD & driver) 
    : driver(driver),
      frame_buffer(width, height, true), rasterizer(frame_buffer), width(width), height(height)
    {    
        init();
#ifdef CONFIG_GRAPHICS_PARALLEL_ENABLED  
        thread_pool.submit_with_stop(std::bind (&Renderer::task_render, this, std::placeholders::_1));
#endif
    }
    
    void Renderer::init()
    {
        if (current_scene)
        {
            size_t total_vertices = 0;
            
            auto meshes = current_scene->collect_components<Mesh>();
            
            for (auto mesh : meshes)
            {
                total_vertices += mesh->get_total_vertices();
            }
            
            transformed_vertices.resize (total_vertices);
                display_vertices.resize (total_vertices);
        }
    }

    // Helper genérico de clipping
    template<typename Inside, typename Intersect>
    static std::vector<glm::fvec4> clipAgainstPlane(
        const std::vector<glm::fvec4>& in,
        Inside   inside,
        Intersect intersect
    )
    {
        std::vector<glm::fvec4> out;
        int n = static_cast < int >(in.size());
        out.reserve(n);
        for (int i = 0; i < n; ++i)
        {
            const auto &A = in[i];
            const auto &B = in[(i+1)%n];
            bool inA = inside(A), inB = inside(B);
            if (inA && inB)
            {
                // ambos dentro → guardo B
                out.push_back(B);
            }
            else if (inA && !inB)
            {
                // sale → sólo el punto de cruce
                out.push_back(intersect(A,B));
            }
            else if (!inA && inB)
            {
                // entra → cruce y luego B
                out.push_back(intersect(A,B));
                out.push_back(B);
            }
            // else ambos fuera → nada
        }
        return out;
    }
    
#ifdef CONFIG_GRAPHICS_PAINTER_ALGO_ENABLED
    void Renderer::render()
    {
        if (!current_scene) 
        {
#ifdef CONFIG_GRAPHICS_PARALLEL_ENABLED
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
#endif
            return;
        }
        init();
        rasterizer.clear();

        // 1) Preparar matrices
        Camera *cam = current_scene->get_main_camera();
        Matrix4x4 view       = cam->get_view_matrix();
        Matrix4x4 proj       = cam->get_projection_matrix();
        Matrix4x4 I(1);
        Matrix4x4 screenTransform =
            glm::translate (I, {width * 0.5f, height * 0.5f, 0.f})
          * glm::scale     (I, {width * 0.5f, height * 0.5f, 1.f});

        struct FaceToDraw {
            std::vector<glm::fvec4> poly;  // vértices en clip-space tras clipping
            float                   depth; // profundidad promedio en NDC
            uint16_t color; // color del mesh
        };
#ifdef CONFIG_GRAPHICS_PARALLEL_ENABLED
            thread_pool.sem_mesh_ready.acquire ();
#endif
        auto meshes = current_scene->collect_components<Mesh>();

        // 2) Construir lista GLOBAL de caras visibles de TODOS los meshes
        std::vector<FaceToDraw> drawList;

        for (auto mesh : meshes)
        {
            const auto &srcVerts = mesh->get_vertices();
            Matrix4x4 model = mesh->get_transform_matrix();
            Matrix4x4 viewModel = view * model;
            Matrix4x4 clipM  = proj * viewModel;

            // Pre-transformar todos los vértices a clip-space
            std::vector<glm::fvec4> clipVerts(srcVerts.size());
            for (size_t i = 0; i < srcVerts.size(); ++i)
                clipVerts[i] = clipM * srcVerts[i];

            // View-space normal matrix: normals change as camera orbits
            glm::mat3 viewNormalMatrix = glm::transpose(glm::inverse(glm::mat3(viewModel)));

            for (auto &face : mesh->get_faces())
            {
                // Back-face culling en view-space
                glm::fvec4 Av = viewModel * srcVerts[face.v1];
                glm::fvec4 Bv = viewModel * srcVerts[face.v2];
                glm::fvec4 Cv = viewModel * srcVerts[face.v3];
                glm::fvec3 A3(Av.x, Av.y, Av.z), B3(Bv.x, Bv.y, Bv.z), C3(Cv.x, Cv.y, Cv.z);
                glm::fvec3 normal = glm::cross(B3 - A3, C3 - A3);
                if (normal.z <= 0.0f) continue;

                // Per-face diffuse lighting in view space (evolves as camera orbits)
                glm::fvec3 face_normal_local = glm::cross(
                    glm::fvec3(srcVerts[face.v2]) - glm::fvec3(srcVerts[face.v1]),
                    glm::fvec3(srcVerts[face.v3]) - glm::fvec3(srcVerts[face.v1])
                );
                glm::fvec3 view_normal = glm::normalize(viewNormalMatrix * face_normal_local);
                float intensity = compute_diffuse_intensity(view_normal, light);
                uint16_t lit_color = apply_light_rgb565(mesh->get_color(), intensity);

                // 2.1) Inicializar polígono en clip-space
                std::vector<glm::fvec4> poly = {
                    clipVerts[face.v1],
                    clipVerts[face.v2],
                    clipVerts[face.v3]
                };
                if (face.is_quad)
                    poly.push_back(clipVerts[face.v4]);

                // 2.2) Sutherland–Hodgman clipping en los 6 planos
                auto clipAndLog = [&](auto inside, auto intersect, const char* plane)
                {
                    size_t before = poly.size();
                    poly = clipAgainstPlane(poly, inside, intersect);
                    size_t after  = poly.size();
                    logger.Log(RENDERER_TAG, 3, "  Clip %s: %zu→%zu vértices", plane, before, after);
                };
                clipAndLog([](auto &v){ return v.x >= -v.w; },
                           [](auto &A, auto &B){ float t=(A.w+A.x)/((A.w+A.x)-(B.w+B.x)); return A + t*(B - A); }, "Left");
                clipAndLog([](auto &v){ return v.x <=  v.w; },
                           [](auto &A, auto &B){ float t=(A.w-A.x)/((A.w-A.x)-(B.w-B.x)); return A + t*(B - A); }, "Right");
                clipAndLog([](auto &v){ return v.y >= -v.w; },
                           [](auto &A, auto &B){ float t=(A.w+A.y)/((A.w+A.y)-(B.w+B.y)); return A + t*(B - A); }, "Bottom");
                clipAndLog([](auto &v){ return v.y <=  v.w; },
                           [](auto &A, auto &B){ float t=(A.w-A.y)/((A.w-A.y)-(B.w-B.y)); return A + t*(B - A); }, "Top");
                clipAndLog([](auto &v){ return v.z >= -v.w; },
                           [](auto &A, auto &B){ float t=(A.w+A.z)/((A.w+A.z)-(B.w+B.z)); return A + t*(B - A); }, "Near");
                clipAndLog([](auto &v){ return v.z <=  v.w; },
                           [](auto &A, auto &B){ float t=(A.w-A.z)/((A.w-A.z)-(B.w-B.z)); return A + t*(B - A); }, "Far");

                if (poly.size() < 3) continue;

                // 2.3) Calcular profundidad promedio en NDC tras clipping
                float depth = 0.0f;
                for (auto &v : poly)
                    depth += (v.z / v.w);
                depth /= float(poly.size());

                drawList.push_back({ std::move(poly), depth, lit_color });
            }
        }

        // 3) Ordenar GLOBALMENTE de mayor a menor depth (más lejano primero)
        std::sort(drawList.begin(), drawList.end(),
                  [](auto &a, auto &b){ return a.depth > b.depth; });

        // 4) Rasterizar en orden
        for (auto &fd : drawList)
        {
            // 4.1) Proyección a pantalla
            std::vector<glm::ivec4> screenPoly;
            screenPoly.reserve(fd.poly.size());
            for (auto &v : fd.poly)
            {
                glm::fvec4 ndc = v / v.w;
                glm::vec4  scr = screenTransform * glm::vec4(ndc.x, ndc.y, ndc.z, 1.f);
                screenPoly.emplace_back(int(scr.x), int(scr.y), int(ndc.z * 1e8f), 1);
            }
            
            // Después de llenar `screenPoly`:
            auto area2 = 0.f;
            for (size_t i = 0, n = screenPoly.size(); i < n; ++i)
            {
                auto &A = screenPoly[i];
                auto &B = screenPoly[(i+1)%n];
                area2 += float(A.x)*B.y - float(A.y)*B.x;
            }
            
            if (std::abs(area2) < 1e-2f)
                continue;  // polígono degenerado, no rasterizar

            rasterizer.set_color(fd.color);
            // 4.2) Llamada al rasterizer
            if (screenPoly.size() == 4)
            {
                face_t q{ true, 0,1,2,3 };
                rasterizer.fill_convex_polygon(screenPoly.data(), &q);
            }
            else if (screenPoly.size() == 3)
            {
                face_t t{ false, 0,1,2,0 };
                rasterizer.fill_convex_polygon(screenPoly.data(), &t);
            }
            else
            {
                for (size_t i = 1; i + 1 < screenPoly.size(); ++i)
                {
                    face_t t{ false, 0, int(i), int(i + 1), 0 };
                    rasterizer.fill_convex_polygon(screenPoly.data(), &t);
                }
            }
        }
        #ifdef CONFIG_GRAPHICS_PARALLEL_ENABLED
            thread_pool.sem_render_done.release();
        #endif

        logger.Log (RENDERER_TAG, 3, "Enviando framebuffer al driver");
        esp_err_t result = driver.send_frame_buffer(frame_buffer.get_buffer());
        
        if (result == ESP_OK)
        {
            logger.Log (RENDERER_TAG, 3, "Framebuffer enviado correctamente");
        }
        else
        {
            logger.Log (RENDERER_TAG, 3, "Error al enviar framebuffer: %s", esp_err_to_name(result));
        }

        frame_buffer.swap_buffers();
    }
#else
    void Renderer::render()
    {
        if (!current_scene) 
        { 
#ifdef CONFIG_GRAPHICS_PARALLEL_ENABLED
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
#endif
            return; 
        } 

        init ();
        rasterizer.clear();

        // 1) Preparar matrices
        Camera *cam = current_scene->get_main_camera();
        Matrix4x4 view       = cam->get_view_matrix();
        Matrix4x4 proj       = cam->get_projection_matrix();
        Matrix4x4 I(1);
        Matrix4x4 screenTransform =
            glm::translate(I, {width*0.5f, height*0.5f, 0.f})
            * glm::scale     (I, {width*0.5f, height*0.5f, 1.f});

#ifdef CONFIG_GRAPHICS_PARALLEL_ENABLED
        thread_pool.sem_mesh_ready.acquire();
#endif
        auto meshes = current_scene->collect_components<Mesh>();
        logger.Log(RENDERER_TAG, 2, "Meshes en escena: %zu", meshes.size());

        for (auto mesh : meshes)
        {
            const auto &srcVerts = mesh->get_vertices();
            logger.Log(RENDERER_TAG, 2, "Mesh tiene %zu vértices y %zu caras",
                        srcVerts.size(), mesh->get_faces().size());
            
            // 2) Transformar a clip-space
            Matrix4x4 model = mesh->get_transform_matrix();
            Matrix4x4 viewModel = view * model;
            Matrix4x4 clipM = proj * viewModel;
            glm::mat3 viewNormalMatrix = glm::transpose(glm::inverse(glm::mat3(viewModel)));
            std::vector<glm::fvec4> clipVerts;
            clipVerts.reserve(srcVerts.size());
            for (size_t i = 0; i < srcVerts.size(); ++i)
            {
                glm::fvec4 cv = clipM * srcVerts[i];
                clipVerts.push_back(cv);
                logger.Log(RENDERER_TAG, 3, "Clip V[%zu] = (%.2f,%.2f,%.2f,%.2f)",
                            i, cv.x, cv.y, cv.z, cv.w);
            }

            // 3) Para cada cara: clipping, pantalla, raster
            for (const auto &face : mesh->get_faces())
            {
                // Per-face diffuse lighting in view space
                glm::fvec3 face_normal_local = glm::cross(
                    glm::fvec3(srcVerts[face.v2]) - glm::fvec3(srcVerts[face.v1]),
                    glm::fvec3(srcVerts[face.v3]) - glm::fvec3(srcVerts[face.v1])
                );
                glm::fvec3 view_normal = glm::normalize(viewNormalMatrix * face_normal_local);
                float intensity = compute_diffuse_intensity(view_normal, light);
                uint16_t lit_color = apply_light_rgb565(mesh->get_color(), intensity);
                rasterizer.set_color(lit_color);
                
                // 3.1 Montar el polígono inicial
                std::vector<glm::fvec4> poly {
                    clipVerts[face.v1],
                    clipVerts[face.v2],
                    clipVerts[face.v3]
                };
                if (face.is_quad) poly.push_back(clipVerts[face.v4]);
                logger.Log(RENDERER_TAG, 3, "Polígono inicial (%zu vértices)", poly.size());

                // 3.2 Clipping en los 6 planos
                auto clipAndLog = [&](auto inside, auto intersect, const char* planeName)
                {
                    size_t before = poly.size();
                    poly = clipAgainstPlane(poly, inside, intersect);
                    size_t after = poly.size();
                    logger.Log(RENDERER_TAG, 3, "  Clip %s: %zu→%zu vértices",
                                planeName, before, after);
                };
                
                clipAndLog(
                    [](auto &v){ return v.x >= -v.w; },
                    [](auto &A,auto &B){ float t=(A.w+A.x)/((A.w+A.x)-(B.w+B.x)); return A+t*(B-A); },
                    "Left");
                clipAndLog(
                    [](auto &v){ return v.x <=  v.w; },
                    [](auto &A,auto &B){ float t=(A.w-A.x)/((A.w-A.x)-(B.w-B.x)); return A+t*(B-A); },
                    "Right");
                clipAndLog(
                    [](auto &v){ return v.y >= -v.w; },
                    [](auto &A,auto &B){ float t=(A.w+A.y)/((A.w+A.y)-(B.w+B.y)); return A+t*(B-A); },
                    "Bottom");
                clipAndLog(
                    [](auto &v){ return v.y <=  v.w; },
                    [](auto &A,auto &B){ float t=(A.w-A.y)/((A.w-A.y)-(B.w-B.y)); return A+t*(B-A); },
                    "Top");
                clipAndLog(
                    [](auto &v){ return v.z >= -v.w; },
                    [](auto &A,auto &B){ float t=(A.w+A.z)/((A.w+A.z)-(B.w+B.z)); return A+t*(B-A); },
                    "Near");
                clipAndLog(
                    [](auto &v){ return v.z <=  v.w; },
                    [](auto &A,auto &B){ float t=(A.w-A.z)/((A.w-A.z)-(B.w-B.z)); return A+t*(B-A); },
                    "Far");

                if (poly.size() < 3)
                {
                    logger.Log(RENDERER_TAG, 3, "  → Descartada por tener %zu vértices tras clipping", poly.size());
                    continue;
                }

                // 3.3 A pantalla
                std::vector<glm::ivec4> screenPoly;
                screenPoly.reserve(poly.size());
                for (size_t i = 0; i < poly.size(); ++i) {
                    glm::fvec4 ndc = poly[i] / poly[i].w;
                    glm::vec4 scr = screenTransform * glm::vec4(ndc.x, ndc.y, ndc.z, 1.f);
                    screenPoly.emplace_back(
                        int(scr.x), int(scr.y), int(ndc.z * 1e8f), 1
                    );
                    logger.Log(RENDERER_TAG, 3,
                                "  Screen[%zu] = (%d,%d) depth=%.2f",
                                i, screenPoly.back().x, screenPoly.back().y, ndc.z);
                }
                logger.Log(RENDERER_TAG, 3, "  screenPoly size = %zu", screenPoly.size());

                // 3.4 Rasterizar
                if (screenPoly.size() == 4)
                {
                    logger.Log(RENDERER_TAG, 3, "  Rasterizando quad");
                    face_t q{ true, 0,1,2,3 };
                    rasterizer.fill_convex_polygon (screenPoly.data(), &q);
                }
                else if (screenPoly.size() == 3)
                {
                    logger.Log(RENDERER_TAG, 3, "  Rasterizando tri");
                    face_t t{ false, 0,1,2,0 };
                    rasterizer.fill_convex_polygon (screenPoly.data(), &t);
                }
                else
                {
                    logger.Log(RENDERER_TAG, 3, "  Rasterizando %zu-tri fan", screenPoly.size()-2);
                    for (size_t i = 1; i+1 < screenPoly.size(); ++i)
                    {
                        face_t t{ false, 0, int(i), int(i+1), 0 };
                        rasterizer.fill_convex_polygon (screenPoly.data(), &t);
                    }
                }
            }
        }

#ifdef CONFIG_GRAPHICS_PARALLEL_ENABLED
        thread_pool.sem_render_done.release();
#endif
        esp_err_t result = driver.send_frame_buffer(frame_buffer.get_buffer());
        
        if (result == ESP_OK)
        {
            logger.Log (RENDERER_TAG, 3, "Framebuffer enviado correctamente");
        }
        else
        {
            logger.Log (RENDERER_TAG, 3, "Error al enviar framebuffer: %s", esp_err_to_name(result));
        }

        frame_buffer.swap_buffers();
    }
#endif
    
    void Renderer::task_render (std::stop_token stop_token)
    {
        while (not running)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Espera activa para evitar saturar el CPU
        }

        unsigned frame_count = 0;

        std::chrono::high_resolution_clock::time_point last_tick = std::chrono::high_resolution_clock::now();
        std::chrono::high_resolution_clock::time_point current_tick;
        std::chrono::high_resolution_clock::duration elapsed_time;

        while (!stop_token.stop_requested())
        {
            current_tick = std::chrono::high_resolution_clock::now();
            render();
            
            frame_count++;

            elapsed_time = current_tick - last_tick;
            last_tick = current_tick;
#if defined(ESP_PLATFORM) && ESP_PLATFORM == 1
            vTaskDelay(1); // Yield to IDLE task to feed the Task WDT
#endif
        }

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

    
} // namespace Ragot
