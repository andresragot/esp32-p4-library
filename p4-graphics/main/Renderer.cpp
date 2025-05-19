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
#include "Logger.hpp"
#include <memory>
#include <chrono>
#include <thread>

#define PAINTER

namespace Ragot
{
    using Matrix4x4 = glm::mat4;
    using glm::fvec4;
    static const char* RENDERER_TAG = "Renderer";
    
#if ESP_PLATFORM != 1
    
    const string Renderer::vertex_shader_code =
    "#version 330\n"
    "layout(location = 0) in vec2 aPos;"   // [-1..+1]
    "layout(location = 1) in vec2 aTex;"   // [0..1]
    ""
    "out vec2 vTex;"
    ""
    "void main()"
    "{"
    "   vTex = aTex;"
    "   gl_Position = vec4(aPos, 0.0, 1.0);"
    "}";
    
    const string Renderer::fragment_shader_code =
    "#version 330\n"
    "in vec2 vTex;"
    "out vec4 FragColor;"
    ""
    "uniform sampler2D uSampler;"
    ""
    "void main()"
    "{"
    "    FragColor = texture(uSampler, vTex);"
    "}";

    
#endif

    Renderer::Renderer (unsigned width, unsigned height)
    :
    #if ESP_PLATFORM == 1
        driver(), 
    #endif
        frame_buffer(width, height, true), rasterizer(frame_buffer), width(width), height(height)
    {
#if ESP_PLATFORM != 1
         quadShader = std::make_unique<Shader_Program>  (
                                                            std::vector<std::string> { vertex_shader_code   },
                                                            std::vector<std::string> { fragment_shader_code }
                                                        );
        
        initFullScreenQuad();
#endif
    }
    
    void Renderer::init()
    {
        if (current_scene)
        {
            size_t total_vertices = 0;
            
            std::vector < Mesh * > meshes = current_scene->collect_components<Mesh>();
            
            for (auto mesh : meshes)
            {
                total_vertices += mesh->get_total_vertices();
            }
            
            transformed_vertices.resize (total_vertices);
                display_vertices.resize (total_vertices);
        }
    }
    
    /*void Renderer::render()
    {
        if (iterations == number_of_iterations)
        {
            std::cout << "Rasterization took " << (accumulated_time / number_of_iterations) << " seconds." << std::endl;
        }
        else if (iterations++ < number_of_iterations)
        {
            auto start = std::chrono::high_resolution_clock::now ();
        
            logger.Log (RENDERER_TAG, 3, "==== Iniciando render() ====");
            
            if (current_scene)
            {
                logger.Log (RENDERER_TAG, 3, "Escena actual encontrada, obteniendo cámara principal");
                Camera * main_camera = current_scene->get_main_camera();
                
                logger.Log (CAMERA_TAG, 3, "Cámara: posición=(%.2f, %.2f, %.2f), target=(%.2f, %.2f, %.2f), FOV=%.1f°",
                         main_camera->get_location().x, main_camera->get_location().y, main_camera->get_location().z,
                         main_camera->get_target().x, main_camera->get_target().y, main_camera->get_target().z,
                         main_camera->get_fov());
                
                std::vector < Mesh * > meshes = current_scene->collect_components<Mesh>();
                logger.Log (RENDERER_TAG, 3, "Encontrados %zu meshes en la escena", meshes.size());

                bool meshes_dirty = false;
                for (const auto mesh : meshes)
                {
                    if (mesh->is_dirty())
                    {
                        mesh->recalculate();
                        meshes_dirty = true;
                    }
                }
            
                if (main_camera->is_dirty() || meshes_dirty)
                {
                    init();
                
                    Matrix4x4 view = main_camera->get_view_matrix();
                    logger.Log (RENDERER_TAG, 3, "Matriz de vista obtenida");

                    Matrix4x4 projection = main_camera->get_projection_matrix();
                    logger.Log (RENDERER_TAG, 3, "Matriz de proyección obtenida (aspect ratio: %.3f)",
                            main_camera->get_aspect_ratio());

                    Matrix4x4 transformation;

                    Matrix4x4 identity_model(1);
                    Matrix4x4 model_base =
                            glm::translate(identity_model, glm::fvec3{0,0,-10})    // z = –10
                          * glm::scale    (identity_model, glm::fvec3{0.75f});     // escala ¾


                    logger.Log (RENDERER_TAG, 3, "Limpiando rasterizador");
                    // Se borra el framebúffer y se dibujan los triángulos:

                    rasterizer.clear ();
                            
                    int total_vertices_transformed = 0;
                    for (auto mesh : meshes)
                    {
                        // Luego, por cada mesh:
                        Matrix4x4 model      = model_base * mesh->get_transform_matrix();
                        transformation  = projection * view * model;
                        // transformation = projection * view * mesh->get_transform_matrix();

                        // Recalculamos los vertices que se verán ahora.
                        mesh->recalculate();

                        logger.Log (MESH_TAG, 3, "Procesando mesh con %zu vértices, posición=(%.2f, %.2f, %.2f)",
                                mesh->get_total_vertices(),
                                mesh->get_position().x, mesh->get_position().y, mesh->get_position().z);
                        
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
                            {
                                logger.Log (TRANSFORM_TAG, 1, "Vértice %zu transformado: (%.2f, %.2f, %.2f, %.2f)",
                                        index, vertex[0], vertex[1], vertex[2], vertex[3]);
                            }
                        }
                        total_vertices_transformed += mesh->get_total_vertices();
                    }
                    logger.Log (RENDERER_TAG, 3, "Total de vértices transformados: %d", total_vertices_transformed);
                    
                    logger.Log (RENDERER_TAG, 3, "Transformando a coordenadas de pantalla (width=%u, height=%u)", width, height);
                    Matrix4x4 identity(1);
                    Matrix4x4     scaling = glm::scale    (identity, glm::fvec3 (width * 0.5f, height * 0.5f, 1.f));
                    Matrix4x4 translation = glm::translate(identity, glm::fvec3 (width * 0.5f, height * 0.5f, 0.f));
                    transformation = translation * scaling;
                            
                    
                    logger.Log (RENDERER_TAG, 3, "Aplicando transformación de pantalla a %zu vértices", transformed_vertices.size());
                    // Transformación correcta de NDC a coordenadas de pantalla
                    for (size_t index = 0; index < transformed_vertices.size(); ++index)
                    {
                        // Los vértices ya están en espacio NDC (-1 a 1)
                        float x = transformed_vertices[index].x;
                        float y = transformed_vertices[index].y;
                        glm::vec4 ndc (x, y, 0, 1);
                        glm::vec4 scr = transformation * ndc;
                        
                        // Transforma de NDC a coordenadas de pantalla
                        display_vertices[index].x = int(scr.x);
                        display_vertices[index].y = int(scr.y);
                        display_vertices[index].z = int(transformed_vertices[index].z * 100000000.0f);
                        display_vertices[index].w = 1.0f;
                        
                        logger.Log (TRANSFORM_TAG, 3, "NDC (%.2f, %.2f) → Pantalla (%d, %d)",
                                x, y, (int)display_vertices[index].x, (int)display_vertices[index].y);
                    }
                    
                    for (const face_t & face : mesh->get_faces())
                    {
                        std::vector < glm::fvec4 > poly;
                        poly.reserve(4);
                        poly.push_back ( display_vertices [face.v1] );
                        poly.push_back ( display_vertices [face.v2] );
                        poly.push_back ( display_vertices [face.v3] );
                        if (face.is_quad) poly.push_back( display_vertices [face.v4] );
                        
                        poly = clipAgainstPlane(poly,
                            [](auto &v){ return v.x >= -v.w; },
                            [](auto &A, auto &B){
                                float t = (A.w + A.x) / ((A.w + A.x) - (B.w + B.x));
                                return A + t*(B - A);
                            });
                        poly = clipAgainstPlane(poly,
                            [](auto &v){ return v.x <=  v.w; },
                            [](auto &A, auto &B){
                                float t = (A.w - A.x) / ((A.w - A.x) - (B.w - B.x));
                                return A + t*(B - A);
                            });
                        poly = clipAgainstPlane(poly,
                            [](auto &v){ return v.y >= -v.w; },
                            [](auto &A, auto &B){
                                float t = (A.w + A.y) / ((A.w + A.y) - (B.w + B.y));
                                return A + t*(B - A);
                            });
                        poly = clipAgainstPlane(poly,
                            [](auto &v){ return v.y <=  v.w; },
                            [](auto &A, auto &B){
                                float t = (A.w - A.y) / ((A.w - A.y) - (B.w - B.y));
                                return A + t*(B - A);
                            });
                        poly = clipAgainstPlane(poly,
                            [](auto &v){ return v.z >= -v.w; },
                            [](auto &A, auto &B){
                                float t = (A.w + A.z) / ((A.w + A.z) - (B.w + B.z));
                                return A + t*(B - A);
                            });
                        poly = clipAgainstPlane(poly,
                            [](auto &v){ return v.z <=  v.w; },
                            [](auto &A, auto &B){
                                float t = (A.w - A.z) / ((A.w - A.z) - (B.w - B.z));
                                return A + t*(B - A);
                            });
                            
                        if (poly.size() < 3) continue;
                        
                        std::vector < fvec4 > screen_poly;
                        screen_poly.reserve(poly.size());
                        for (auto &v_clip : poly)
                        {
                            fvec4 v_ndc = v_clip / v_clip.w;
                            fvec4 v_screen = transformation * v_ndc;
                        }
                    }

                    logger.Log (RENDERER_TAG, 3, "Iniciando renderizado de triángulos");
                    int frontfaces = 0, backfaces = 0;
                    for (auto mesh : meshes)
                    {
                        const face_t * indices = mesh->get_faces().data();
                        const face_t * end = indices + mesh->get_faces().size();
                        logger.Log (RENDERER_TAG, 3, "Procesando mesh con %zu caras", mesh->get_faces().size());
                        
                        for (; indices < end; )
                        {
                            if (is_frontface(transformed_vertices.data(), indices))
                            {
                                frontfaces++;
                                rasterizer.set_color(RGB565(0x000FF));
                                
                                // if (frontfaces % 10 == 0) // Log solo algunas caras para no saturar
                                {
                                    logger.Log (TRIANGLE_TAG, 3, "Rasterizando cara %d: v1=%d (%.1f,%.1f), v2=%d (%.1f,%.1f), v3=%d (%.1f,%.1f)",
                                            frontfaces,
                                            indices->v1, display_vertices[indices->v1][0], display_vertices[indices->v1][1],
                                            indices->v2, display_vertices[indices->v2][0], display_vertices[indices->v2][1],
                                            indices->v3, display_vertices[indices->v3][0], display_vertices[indices->v3][1]);
                                }
                                
                                rasterizer.fill_convex_polygon_z_buffer (display_vertices.data(), indices);
                            }
                            else
                            {
                                backfaces++;
                            }
                            
                            ++indices;
                        }
                    }
                    logger.Log (RENDERER_TAG, 3, "Total caras procesadas: %d frontfaces, %d backfaces", frontfaces, backfaces);

                    logger.Log (RENDERER_TAG, 3, "Enviando framebuffer al driver");
                    #if ESP_PLATFORM == 1
                    esp_err_t result = driver.send_frame_buffer(frame_buffer.get_buffer());
                    
                    if (result == ESP_OK)
                    {
                        logger.Log (RENDERER_TAG, 3, "Framebuffer enviado correctamente");
                    }
                    else
                    {
                        logger.Log (RENDERER_TAG, 3, "Error al enviar framebuffer: %s", esp_err_to_name(result));
                    }
                    #else
                    // En Renderer::render(), en la sección #else de ESP_PLATFORM
                    frame_buffer.sendGL();

                    // Limpia la pantalla
                    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
                    glClear(GL_COLOR_BUFFER_BIT);

                    // Dibuja el quad
                    quadShader->use();
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, frame_buffer.getGLTex());
                    GLuint loc = quadShader->get_uniform_location("uSampler");
                    glUniform1i(loc, 0);

                    glBindVertexArray(quadVAO);
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
                    glBindVertexArray(0);

                    // Swap de buffers y limpieza de tu framebuffer
                    frame_buffer.swap_buffers();
                    frame_buffer.clear_buffer();

                    
                    #endif
                    
                    logger.Log (RENDERER_TAG, 3, "Swapping y limpiando buffers");
                    frame_buffer.swap_buffers();
                    frame_buffer.clear_buffer();
                }
                else
                {
                    logger.Log(RENDERER_TAG, 3, "Not diry");
                    frame_buffer.blit_to_window();
                    #if ESP_PLATFORM == 1
                    driver.send_frame_buffer(frame_buffer.get_buffer());
                    #else
                    frame_buffer.sendGL();
                    #endif
                    frame_buffer.swap_buffers();
                }
            }

            logger.Log (RENDERER_TAG, 3, "==== Renderizado completado ====\n");
            
            auto end = std::chrono::high_resolution_clock::now ();
            
            auto elapsed_seconds = std::chrono::duration < float > (end - start).count();
            
            accumulated_time += elapsed_seconds;
        }
    }*/
    
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
    
#ifdef PAINTER
    void Renderer::render()
    {
        if (!current_scene) return;
        init();
        rasterizer.clear();
        rasterizer.set_color(RGB565(0x000FF));

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
        };

        auto meshes = current_scene->collect_components<Mesh>();
        for (auto *mesh : meshes)
        {
            mesh->recalculate();
            const auto &srcVerts = mesh->get_vertices();
            Matrix4x4 model = mesh->get_transform_matrix();
            Matrix4x4 clipM  = proj * view * model;

            // Pre-transformar todos los vértices a clip-space
            std::vector<glm::fvec4> clipVerts(srcVerts.size());
            for (size_t i = 0; i < srcVerts.size(); ++i)
                clipVerts[i] = clipM * srcVerts[i];

            // 2) Construir lista de caras visibles + polígonos recortados + depth
            std::vector<FaceToDraw> drawList;
            drawList.reserve(mesh->get_faces().size());

            for (auto &face : mesh->get_faces())
            {
                // Back-face culling en view-space
                glm::fvec4 Av = view * model * srcVerts[face.v1];
                glm::fvec4 Bv = view * model * srcVerts[face.v2];
                glm::fvec4 Cv = view * model * srcVerts[face.v3];
                glm::fvec3 A3(Av.x, Av.y, Av.z), B3(Bv.x, Bv.y, Bv.z), C3(Cv.x, Cv.y, Cv.z);
                glm::fvec3 normal = glm::cross(B3 - A3, C3 - A3);
                if (normal.z >= 0.0f) continue;

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

                drawList.push_back({ std::move(poly), depth });
            }

            // 3) Ordenar de menor a mayor depth (más lejano primero)
            std::sort(drawList.begin(), drawList.end(),
                      [](auto &a, auto &b){ return a.depth < b.depth; });

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
        }

        logger.Log (RENDERER_TAG, 3, "Enviando framebuffer al driver");
        #if ESP_PLATFORM == 1
        esp_err_t result = driver.send_frame_buffer(frame_buffer.get_buffer());
        
        if (result == ESP_OK)
        {
            logger.Log (RENDERER_TAG, 3, "Framebuffer enviado correctamente");
        }
        else
        {
            logger.Log (RENDERER_TAG, 3, "Error al enviar framebuffer: %s", esp_err_to_name(result));
        }
        #else
        // En Renderer::render(), en la sección #else de ESP_PLATFORM
        frame_buffer.sendGL();

        // Limpia la pantalla
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Dibuja el quad
        quadShader->use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, frame_buffer.getGLTex());
        GLuint loc = quadShader->get_uniform_location("uSampler");
        glUniform1i(loc, 0);

        glBindVertexArray(quadVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);

        // Swap de buffers y limpieza de tu framebuffer
        frame_buffer.swap_buffers();
        frame_buffer.clear_buffer();

        
        #endif
        
        logger.Log (RENDERER_TAG, 3, "Swapping y limpiando buffers");
        frame_buffer.swap_buffers();
        frame_buffer.clear_buffer();
    }
#else
    void Renderer::render()
    {
        if (!current_scene) return;
        init();
        rasterizer.clear();
        rasterizer.set_color(RGB565(0x000FF));

        // 1) Preparar matrices
        Camera *cam = current_scene->get_main_camera();
        Matrix4x4 view       = cam->get_view_matrix();
        Matrix4x4 proj       = cam->get_projection_matrix();
        Matrix4x4 I(1);
        Matrix4x4 screenTransform =
            glm::translate(I, {width*0.5f, height*0.5f, 0.f})
          * glm::scale     (I, {width*0.5f, height*0.5f, 1.f});

        auto meshes = current_scene->collect_components<Mesh>();
        logger.Log(RENDERER_TAG, 2, "Meshes en escena: %zu", meshes.size());

        for (auto *mesh : meshes)
        {
            mesh->recalculate();
            const auto &srcVerts = mesh->get_vertices();
            logger.Log(RENDERER_TAG, 2, "Mesh tiene %zu vértices y %zu caras",
                       srcVerts.size(), mesh->get_faces().size());
            
            // 2) Transformar a clip-space
            Matrix4x4 model = mesh->get_transform_matrix();
            Matrix4x4 clipM = proj * view * model;
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

        logger.Log (RENDERER_TAG, 3, "Enviando framebuffer al driver");
        #if ESP_PLATFORM == 1
        esp_err_t result = driver.send_frame_buffer(frame_buffer.get_buffer());
        
        if (result == ESP_OK)
        {
            logger.Log (RENDERER_TAG, 3, "Framebuffer enviado correctamente");
        }
        else
        {
            logger.Log (RENDERER_TAG, 3, "Error al enviar framebuffer: %s", esp_err_to_name(result));
        }
        #else
        // En Renderer::render(), en la sección #else de ESP_PLATFORM
        frame_buffer.sendGL();

        // Limpia la pantalla
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Dibuja el quad
        quadShader->use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, frame_buffer.getGLTex());
        GLuint loc = quadShader->get_uniform_location("uSampler");
        glUniform1i(loc, 0);

        glBindVertexArray(quadVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);

        // Swap de buffers y limpieza de tu framebuffer
        frame_buffer.swap_buffers();
        frame_buffer.clear_buffer();

        
        #endif
        
        logger.Log (RENDERER_TAG, 3, "Swapping y limpiando buffers");
        frame_buffer.swap_buffers();
        frame_buffer.clear_buffer();
    }
#endif
    
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

#if ESP_PLATFORM != 1
    void Renderer::initFullScreenQuad()
    {
        // Define vértices (pos XY + UV)
        float quadVerts[] =
        {
            -1.f, -1.f, 0.f, 0.f,
             1.f, -1.f, 1.f, 0.f,
             1.f,  1.f, 1.f, 1.f,
            -1.f,  1.f, 0.f, 1.f,
        };
        unsigned quadIdx[] = { 0,1,2, 2,3,0 };
        
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glGenBuffers(1, &quadEBO);
        glBindVertexArray(quadVAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIdx), quadIdx, GL_STATIC_DRAW);
        
        // posición = layout(location=0)
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // UV = layout(location=1)
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
        glEnableVertexAttribArray(1);
        
        glBindVertexArray(0);
    }
#endif
    
} // namespace Ragot
