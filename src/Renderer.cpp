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
#ifdef CONFIG_GRAPHICS_DEBUG_OVERLAY
      , debug_overlay(rasterizer, width, height)
#endif
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

    // In-place Sutherland-Hodgman clipping against a single plane that writes
    // into the caller-provided output buffer. Avoids heap allocations and the
    // per-plane copy of the original helper.
    template<typename Inside, typename Intersect>
    static inline void clipAgainstPlaneInto(
        const std::vector<glm::fvec4>& in,
        std::vector<glm::fvec4>&       out,
        Inside    inside,
        Intersect intersect)
    {
        out.clear();
        const int n = static_cast<int>(in.size());
        if (n == 0) return;
        out.reserve(n + 2);
        for (int i = 0; i < n; ++i)
        {
            const glm::fvec4 & A = in[i];
            const glm::fvec4 & B = in[(i + 1) % n];
            const bool inA = inside(A), inB = inside(B);
            if (inA && inB)              out.push_back(B);
            else if (inA && !inB)        out.push_back(intersect(A, B));
            else if (!inA && inB)      { out.push_back(intersect(A, B)); out.push_back(B); }
        }
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

#ifdef CONFIG_GRAPHICS_DEBUG_OVERLAY
        auto t_frame_start = std::chrono::high_resolution_clock::now();
        debug_overlay.begin_frame();
#endif

        // 1) Preparar matrices
        Camera *cam = current_scene->get_main_camera();
        Matrix4x4 view       = cam->get_view_matrix();
        Matrix4x4 proj       = cam->get_projection_matrix();
        Matrix4x4 I(1);
        // Flip vertical (Y- en scale) para que NDC y=+1 (arriba del mundo)
        // mapee a la fila 0 del framebuffer (parte alta de la pantalla),
        // coincidiendo con la orientación del HUD.
        Matrix4x4 screenTransform =
            glm::translate (I, {width * 0.5f, height * 0.5f, 0.f})
          * glm::scale     (I, {width * 0.5f, -float(height) * 0.5f, 1.f});

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

#ifdef CONFIG_GRAPHICS_FRUSTUM_CULLING
            // Mesh-level frustum culling vs bounding sphere (Gribb-Hartmann).
            {
                const glm::vec3 & sc = mesh->get_bounding_center();
                float             sr = mesh->get_bounding_radius();
                if (sr > 0.f)
                {
                    auto row = [&](int i) {
                        return glm::vec4(clipM[0][i], clipM[1][i],
                                         clipM[2][i], clipM[3][i]);
                    };
                    glm::vec4 r0 = row(0), r1 = row(1), r2 = row(2), r3 = row(3);
                    glm::vec4 planes[6] = {
                        r3 + r0, r3 - r0,
                        r3 + r1, r3 - r1,
                        r3 + r2, r3 - r2,
                    };
                    bool outside = false;
                    for (auto & p : planes)
                    {
                        glm::vec3 n(p.x, p.y, p.z);
                        float len = glm::length(n);
                        if (len <= 0.f) continue;
                        float dist = (n.x * sc.x + n.y * sc.y + n.z * sc.z + p.w) / len;
                        if (dist < -sr) { outside = true; break; }
                    }
                    if (outside)
                    {
#ifdef CONFIG_GRAPHICS_DEBUG_OVERLAY
                        debug_overlay.note_mesh_culled(mesh->get_faces().size());
#endif
                        continue;
                    }
                }
            }
#endif
#ifdef CONFIG_GRAPHICS_SMALL_OBJECT_CULLING
            {
                const glm::vec3 & sc = mesh->get_bounding_center();
                float             sr = mesh->get_bounding_radius();
                if (sr > 0.f)
                {
                    glm::vec4 vc = viewModel * glm::vec4(sc, 1.f);
                    float depth = -vc.z;
                    if (depth > sr)
                    {
                        float fy = proj[1][1];
                        float px_radius = (sr * fy / depth) * (0.5f * float(height));
                        const float kMin = float(CONFIG_GRAPHICS_SMALL_OBJECT_PIXELS);
                        if (px_radius < kMin)
                        {
#ifdef CONFIG_GRAPHICS_DEBUG_OVERLAY
                            debug_overlay.note_mesh_culled(mesh->get_faces().size());
#endif
                            continue;
                        }
                    }
                }
            }
#endif

            // Pre-transformar todos los vértices a clip-space
            std::vector<glm::fvec4> clipVerts(srcVerts.size());
            for (size_t i = 0; i < srcVerts.size(); ++i)
                clipVerts[i] = clipM * srcVerts[i];

            // View-space normal matrix: normals change as camera orbits
            glm::mat3 viewNormalMatrix = glm::transpose(glm::inverse(glm::mat3(viewModel)));

            for (auto &face : mesh->get_faces())
            {
#ifdef CONFIG_GRAPHICS_DEBUG_OVERLAY
                debug_overlay.note_face_total();
#endif
                // Back-face culling en view-space
                glm::fvec4 Av = viewModel * srcVerts[face.v1];
                glm::fvec4 Bv = viewModel * srcVerts[face.v2];
                glm::fvec4 Cv = viewModel * srcVerts[face.v3];
                glm::fvec3 A3(Av.x, Av.y, Av.z), B3(Bv.x, Bv.y, Bv.z), C3(Cv.x, Cv.y, Cv.z);
                glm::fvec3 normal = glm::cross(B3 - A3, C3 - A3);
                if (normal.z <= 0.0f)
                {
#ifdef CONFIG_GRAPHICS_DEBUG_OVERLAY
                    debug_overlay.note_face_culled();
#endif
                    continue;
                }

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

                if (poly.size() < 3)
                {
#ifdef CONFIG_GRAPHICS_DEBUG_OVERLAY
                    debug_overlay.note_face_clipped();
#endif
                    continue;
                }

                // 2.3) Calcular profundidad promedio en NDC tras clipping
                float depth = 0.0f;
                for (auto &v : poly)
                    depth += (v.z / v.w);
                depth /= float(poly.size());

                drawList.push_back({ std::move(poly), depth, lit_color });

#ifdef CONFIG_GRAPHICS_DEBUG_SHOW_NORMALS
                // Normal viz: línea amarilla desde el centroide en dirección
                // de la normal proyectada. Proyectamos centroide y centroide+N
                // a través de la misma cadena view*model -> clip -> NDC -> screen.
                {
                    glm::fvec3 c_local(0.f);
                    int nv = face.is_quad ? 4 : 3;
                    int idx[4] = { face.v1, face.v2, face.v3, face.v4 };
                    for (int k = 0; k < nv; ++k)
                        c_local += glm::fvec3(srcVerts[idx[k]]);
                    c_local /= float(nv);
                    glm::fvec3 n_local = glm::normalize(face_normal_local) * 0.5f;
                    glm::fvec4 c_clip = clipM * glm::fvec4(c_local, 1.f);
                    glm::fvec4 t_clip = clipM * glm::fvec4(c_local + n_local, 1.f);
                    if (c_clip.w > 0.f && t_clip.w > 0.f)
                    {
                        glm::fvec4 cn = c_clip / c_clip.w;
                        glm::fvec4 tn = t_clip / t_clip.w;
                        glm::vec4 cs = screenTransform * glm::vec4(cn.x, cn.y, cn.z, 1.f);
                        glm::vec4 ts = screenTransform * glm::vec4(tn.x, tn.y, tn.z, 1.f);
                        // RGB565 yellow = 0xFFE0
                        rasterizer.draw_line(int(cs.x), int(cs.y), int(ts.x), int(ts.y), 0xFFE0);
                    }
                }
#endif
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
#ifdef CONFIG_GRAPHICS_DEBUG_WIREFRAME
            // Wireframe: dibujar sólo las aristas.
            for (size_t i = 0, n = screenPoly.size(); i < n; ++i)
            {
                const auto & a = screenPoly[i];
                const auto & b = screenPoly[(i + 1) % n];
                rasterizer.draw_line(a.x, a.y, b.x, b.y, fd.color);
            }
#ifdef CONFIG_GRAPHICS_DEBUG_OVERLAY
            debug_overlay.note_face_drawn();
#endif
#else
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
#ifdef CONFIG_GRAPHICS_DEBUG_OVERLAY
            debug_overlay.note_face_drawn();
#endif
#endif // CONFIG_GRAPHICS_DEBUG_WIREFRAME
        }
        #ifdef CONFIG_GRAPHICS_PARALLEL_ENABLED
            thread_pool.sem_render_done.release();
        #endif

#ifdef CONFIG_GRAPHICS_DEBUG_OVERLAY
        {
            auto t_frame_end = std::chrono::high_resolution_clock::now();
            uint64_t frame_us = std::chrono::duration_cast<std::chrono::microseconds>(
                t_frame_end - t_frame_start).count();
            debug_overlay.end_frame(frame_us);
            debug_overlay.draw();
        }
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

#ifdef CONFIG_GRAPHICS_DEBUG_OVERLAY
        auto t_frame_start = std::chrono::high_resolution_clock::now();
        debug_overlay.begin_frame();
#endif

        // 1) Preparar matrices
        Camera *cam = current_scene->get_main_camera();
        Matrix4x4 view       = cam->get_view_matrix();
        Matrix4x4 proj       = cam->get_projection_matrix();
        Matrix4x4 I(1);
        Matrix4x4 screenTransform =
            glm::translate(I, {width*0.5f, height*0.5f, 0.f})
            * glm::scale     (I, {width*0.5f, -float(height)*0.5f, 1.f});

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
#ifdef CONFIG_GRAPHICS_FRUSTUM_CULLING
            // Frustum culling: extract the 6 clip-space planes from `clipM`
            // (Gribb-Hartmann) and reject the whole mesh if its bounding
            // sphere is fully outside any plane. The sphere is in the same
            // space as `srcVerts`, which is the input space of `clipM`.
            {
                const glm::vec3 & sc = mesh->get_bounding_center();
                float             sr = mesh->get_bounding_radius();
                if (sr > 0.f)
                {
                    auto row = [&](int i) {
                        return glm::vec4(clipM[0][i], clipM[1][i],
                                         clipM[2][i], clipM[3][i]);
                    };
                    glm::vec4 r0 = row(0), r1 = row(1), r2 = row(2), r3 = row(3);
                    glm::vec4 planes[6] = {
                        r3 + r0, // left
                        r3 - r0, // right
                        r3 + r1, // bottom
                        r3 - r1, // top
                        r3 + r2, // near
                        r3 - r2, // far
                    };
                    bool outside = false;
                    for (auto & p : planes)
                    {
                        glm::vec3 n(p.x, p.y, p.z);
                        float     len = glm::length(n);
                        if (len <= 0.f) continue;
                        float dist = (n.x * sc.x + n.y * sc.y + n.z * sc.z + p.w) / len;
                        if (dist < -sr) { outside = true; break; }
                    }
                    if (outside)
                    {
                        logger.Log(RENDERER_TAG, 3, "  → Mesh culled by frustum");
#ifdef CONFIG_GRAPHICS_DEBUG_OVERLAY
                        debug_overlay.note_mesh_culled(mesh->get_faces().size());
#endif
                        continue;
                    }
                }
            }
#endif
#ifdef CONFIG_GRAPHICS_SMALL_OBJECT_CULLING
            // Small-object culling: project the bounding sphere to screen and
            // skip the whole mesh if its on-screen radius would be tiny.
            {
                const glm::vec3 & sc = mesh->get_bounding_center();
                float             sr = mesh->get_bounding_radius();
                if (sr > 0.f)
                {
                    // View-space distance along the camera forward axis
                    // (camera looks down -Z in view space).
                    glm::vec4 vc = viewModel * glm::vec4(sc, 1.f);
                    float depth = -vc.z;
                    if (depth > sr) // not behind near plane / not enclosing camera
                    {
                        // Approx projected radius in pixels using vertical FOV
                        // encoded in the projection matrix: proj[1][1] = 1/tan(fy/2).
                        float fy = proj[1][1];
                        float px_radius = (sr * fy / depth) * (0.5f * float(height));
                        const float kMin = float(CONFIG_GRAPHICS_SMALL_OBJECT_PIXELS);
                        if (px_radius < kMin)
                        {
#ifdef CONFIG_GRAPHICS_DEBUG_OVERLAY
                            debug_overlay.note_mesh_culled(mesh->get_faces().size());
#endif
                            continue;
                        }
                    }
                }
            }
#endif
            glm::mat3 viewNormalMatrix = glm::transpose(glm::inverse(glm::mat3(viewModel)));
            std::vector<glm::fvec4> clipVerts;
            clipVerts.reserve(srcVerts.size());
            // Cache view-space positions too so per-face back-face culling
            // doesn't re-multiply the model matrix three times per face.
            std::vector<glm::fvec3> viewVerts;
            viewVerts.reserve(srcVerts.size());
            for (size_t i = 0; i < srcVerts.size(); ++i)
            {
                glm::fvec4 vv = viewModel * srcVerts[i];
                viewVerts.emplace_back(vv.x, vv.y, vv.z);
                glm::fvec4 cv = clipM * srcVerts[i];
                clipVerts.push_back(cv);
                logger.Log(RENDERER_TAG, 3, "Clip V[%zu] = (%.2f,%.2f,%.2f,%.2f)",
                            i, cv.x, cv.y, cv.z, cv.w);
            }

            // 3) Para cada cara: clipping, pantalla, raster
            for (const auto &face : mesh->get_faces())
            {
#ifdef CONFIG_GRAPHICS_DEBUG_OVERLAY
                debug_overlay.note_face_total();
#endif
                // Back-face culling en view-space (antes de iluminar, clipping,
                // proyección, etc.). Ahorra el grueso del coste por cara descartada.
                {
                    const glm::fvec3 & A3 = viewVerts[face.v1];
                    const glm::fvec3 & B3 = viewVerts[face.v2];
                    const glm::fvec3 & C3 = viewVerts[face.v3];
                    glm::fvec3 face_normal_view = glm::cross(B3 - A3, C3 - A3);
                    // Si la cara mira lejos de la cámara la descartamos.
                    // La cámara mira hacia -Z en view-space, por lo que una cara
                    // visible (CCW) genera normal con z > 0.
                    if (face_normal_view.z <= 0.0f)
                    {
#ifdef CONFIG_GRAPHICS_DEBUG_OVERLAY
                        debug_overlay.note_face_culled();
#endif
                        continue;
                    }
                }

                // Per-face diffuse lighting in view space
                glm::fvec3 face_normal_local = glm::cross(
                    glm::fvec3(srcVerts[face.v2]) - glm::fvec3(srcVerts[face.v1]),
                    glm::fvec3(srcVerts[face.v3]) - glm::fvec3(srcVerts[face.v1])
                );
                glm::fvec3 view_normal = glm::normalize(viewNormalMatrix * face_normal_local);
                float intensity = compute_diffuse_intensity(view_normal, light);
                uint16_t lit_color = apply_light_rgb565(mesh->get_color(), intensity);
                rasterizer.set_color(lit_color);
                
                // 3.1 Montar el polígono inicial en buffers reutilizables
                // (ping-pong) para evitar allocations por cara.
                static thread_local std::vector<glm::fvec4> poly_buf_a;
                static thread_local std::vector<glm::fvec4> poly_buf_b;
                std::vector<glm::fvec4> * pIn  = &poly_buf_a;
                std::vector<glm::fvec4> * pOut = &poly_buf_b;
                pIn->clear();
                pIn->push_back(clipVerts[face.v1]);
                pIn->push_back(clipVerts[face.v2]);
                pIn->push_back(clipVerts[face.v3]);
                if (face.is_quad) pIn->push_back(clipVerts[face.v4]);

                // 3.2 Clipping en los 6 planos (in-place, ping-pong)
                auto clipStep = [&](auto inside, auto intersect)
                {
                    clipAgainstPlaneInto(*pIn, *pOut, inside, intersect);
                    std::swap(pIn, pOut);
                };
                clipStep(
                    [](auto &v){ return v.x >= -v.w; },
                    [](auto &A,auto &B){ float t=(A.w+A.x)/((A.w+A.x)-(B.w+B.x)); return A+t*(B-A); });
                clipStep(
                    [](auto &v){ return v.x <=  v.w; },
                    [](auto &A,auto &B){ float t=(A.w-A.x)/((A.w-A.x)-(B.w-B.x)); return A+t*(B-A); });
                clipStep(
                    [](auto &v){ return v.y >= -v.w; },
                    [](auto &A,auto &B){ float t=(A.w+A.y)/((A.w+A.y)-(B.w+B.y)); return A+t*(B-A); });
                clipStep(
                    [](auto &v){ return v.y <=  v.w; },
                    [](auto &A,auto &B){ float t=(A.w-A.y)/((A.w-A.y)-(B.w-B.y)); return A+t*(B-A); });
                clipStep(
                    [](auto &v){ return v.z >= -v.w; },
                    [](auto &A,auto &B){ float t=(A.w+A.z)/((A.w+A.z)-(B.w+B.z)); return A+t*(B-A); });
                clipStep(
                    [](auto &v){ return v.z <=  v.w; },
                    [](auto &A,auto &B){ float t=(A.w-A.z)/((A.w-A.z)-(B.w-B.z)); return A+t*(B-A); });

                std::vector<glm::fvec4> & poly = *pIn;

                if (poly.size() < 3)
                {
#ifdef CONFIG_GRAPHICS_DEBUG_OVERLAY
                    debug_overlay.note_face_clipped();
#endif
                    continue;
                }

                // 3.3 A pantalla (buffer reusable)
                static thread_local std::vector<glm::ivec4> screenPoly;
                screenPoly.clear();
                screenPoly.reserve(poly.size());
                for (size_t i = 0; i < poly.size(); ++i) {
                    glm::fvec4 ndc = poly[i] / poly[i].w;
                    glm::vec4 scr = screenTransform * glm::vec4(ndc.x, ndc.y, ndc.z, 1.f);
                    screenPoly.emplace_back(
                        int(scr.x), int(scr.y), int(ndc.z * 1e8f), 1
                    );
                }

#ifdef CONFIG_GRAPHICS_DEBUG_WIREFRAME
                // Wireframe: dibuja sólo las aristas con el color iluminado del mesh.
                for (size_t i = 0, n = screenPoly.size(); i < n; ++i)
                {
                    const auto & a = screenPoly[i];
                    const auto & b = screenPoly[(i + 1) % n];
                    rasterizer.draw_line(a.x, a.y, b.x, b.y, lit_color);
                }
#ifdef CONFIG_GRAPHICS_DEBUG_OVERLAY
                debug_overlay.note_face_drawn();
#endif
#else
                // 3.4 Rasterizar (fill convencional)
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
#ifdef CONFIG_GRAPHICS_DEBUG_OVERLAY
                debug_overlay.note_face_drawn();
#endif
#endif // CONFIG_GRAPHICS_DEBUG_WIREFRAME

#ifdef CONFIG_GRAPHICS_DEBUG_SHOW_NORMALS
                // Normal viz: línea amarilla desde el centroide en dirección
                // de la normal proyectada. Proyectamos centroide y centroide+N
                // a través de la misma cadena view*model -> clip -> NDC -> screen.
                {
                    glm::fvec3 c_local(0.f);
                    int nv = face.is_quad ? 4 : 3;
                    int idx[4] = { face.v1, face.v2, face.v3, face.v4 };
                    for (int k = 0; k < nv; ++k)
                        c_local += glm::fvec3(srcVerts[idx[k]]);
                    c_local /= float(nv);
                    glm::fvec3 n_local = glm::normalize(face_normal_local) * 0.5f;
                    glm::fvec4 c_clip = clipM * glm::fvec4(c_local, 1.f);
                    glm::fvec4 t_clip = clipM * glm::fvec4(c_local + n_local, 1.f);
                    if (c_clip.w > 0.f && t_clip.w > 0.f)
                    {
                        glm::fvec4 cn = c_clip / c_clip.w;
                        glm::fvec4 tn = t_clip / t_clip.w;
                        glm::vec4 cs = screenTransform * glm::vec4(cn.x, cn.y, cn.z, 1.f);
                        glm::vec4 ts = screenTransform * glm::vec4(tn.x, tn.y, tn.z, 1.f);
                        // RGB565 yellow = 0xFFE0
                        rasterizer.draw_line(int(cs.x), int(cs.y), int(ts.x), int(ts.y), 0xFFE0);
                    }
                }
#endif
            }
        }

#ifdef CONFIG_GRAPHICS_PARALLEL_ENABLED
        thread_pool.sem_render_done.release();
#endif

#ifdef CONFIG_GRAPHICS_DEBUG_OVERLAY
        {
            auto t_frame_end = std::chrono::high_resolution_clock::now();
            uint64_t frame_us = std::chrono::duration_cast<std::chrono::microseconds>(
                t_frame_end - t_frame_start).count();
            debug_overlay.end_frame(frame_us);
            debug_overlay.draw();
        }
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
