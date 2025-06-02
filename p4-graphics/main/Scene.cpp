/**
 * @file Scene.cpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief Implementation of the Scene class for managing 3D scenes.
 * @details The Scene class provides methods to manage nodes, cameras, and scene traversal.
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

#include "Scene.hpp"
#include "Mesh.hpp"
#include <queue>
#include <algorithm>
#include "Logger.hpp"
#ifdef CONFIG_GRAPHICS_PARALLEL_ENABLED
#include "Thread_Pool.hpp"
#endif

namespace Ragot
{

    Scene::Scene ()
    {
#ifdef CONFIG_GRAPHICS_PARALLEL_ENABLED
        thread_pool.submit_with_stop (std::bind (&Scene::task_update, this, std::placeholders::_1, 0.f));
#endif
        root_node = std::make_shared < Node > ();
    }
    
    Scene::Scene (Camera * camera) : main_camera (camera)
    {
#ifdef CONFIG_GRAPHICS_PARALLEL_ENABLED
        thread_pool.submit_with_stop (std::bind (&Scene::task_update, this, std::placeholders::_1, 0.f));
#endif
        root_node = std::make_shared < Node > ();
    }

    void Scene::add_node (std::shared_ptr < Node > node, const basics::Id name)
    {
        if (!node) return;
        
        root_node->add_child(node);
        
        named_nodes[name] = node;
    }
    
    void Scene::remove_node(std::shared_ptr < Node > node)
    {
        if (!node) return;
        
        // Remove from named nodes if it exists
        for (auto it = named_nodes.begin(); it != named_nodes.end(); ) 
        {
            if (it->second == node) 
            {
                it = named_nodes.erase(it);
            } 
            else 
            {
                ++it;
            }
        }
        
        root_node->remove_child(node);
    }
    
    std::shared_ptr < Node > Scene::find_node(const basics::Id name)
    {
        auto it = named_nodes.find(name);
        return (it != named_nodes.end()) ? it->second : nullptr;
    }
    
    void Scene::set_main_camera(Camera* camera)
    {
        if (camera) 
        {
            main_camera = camera;
        }
    }
    
    void Scene::traverse(const std::function < void (std::shared_ptr < Node >) > & callback)
    {
        if (!callback) return;
        
        std::queue < std::shared_ptr < Node > > node_queue;
        node_queue.push (root_node);
        
        while (!node_queue.empty()) 
        {
            std::shared_ptr < Node > current = node_queue.front();
            node_queue.pop();
            
            callback(current);
            
            // Add all children to the queue
            for (auto & child : current->get_children()) 
            {
                node_queue.push(child);
            }
        }
    }
    
    template<typename T>
    std::vector<std::shared_ptr < T > > Scene::collect_components()
    {
        ///< @todo sacar este vector.
        std::vector< std::shared_ptr < T > > result;
        
        traverse([&result](std::shared_ptr < Node > node) 
        {
            // Check if node is a Component
            auto component = std::dynamic_pointer_cast< Component >(node);
            if (component) 
            {
                // Try to cast to the requested type
                auto typed_component = std::dynamic_pointer_cast < T >(component);
                if (typed_component) 
                {
                    result.emplace_back(typed_component);
                }
                
                // Also check components attached to this component
                for (auto subComponent : component->get_components()) 
                {
                    auto typed_subComponent = std::dynamic_pointer_cast < T >(subComponent);
                    if (typed_subComponent) 
                    {
                        result.emplace_back(typed_subComponent);
                    }
                }
            }
        });
        
        return result;
    }

    void Scene::update(float /*delta_time no usado*/)
    {
        auto meshes = collect_components<Mesh>();
        if (meshes.empty()) return;

        glm::vec3 centro(0.0f);
        for (auto mesh : meshes)
            centro += mesh->get_position();
        centro /= static_cast<float>(meshes.size());

        static float anguloCam = 0.0f;
        const float pasoPorFrame = 0.02f; // radians/frame

        anguloCam += pasoPorFrame;
        if (anguloCam > glm::two_pi<float>())
            anguloCam -= glm::two_pi<float>();

        const float radio = 10.0f;
        const float altura = 3.0f;

        float camX = centro.x + radio * cos(anguloCam);
        float camZ = centro.z + radio * sin(anguloCam);
        float camY = centro.y + altura;

        Camera* cam = get_main_camera();
        if (!cam) return;

        cam->set_location(glm::vec3(camX, camY, camZ));
        cam->set_target(centro);
    }


    // void Scene::update(float delta_time)
    // {
    //     // 1. Recolectamos todas las meshes
    //     auto meshes = collect_components<Mesh>();

    //     // 2. Si no hay meshes, no hay nada que orbitar
    //     if (meshes.empty())
    //         return;

    //     // 3. Calculamos el “centro” como promedio de posiciones
    //     glm::vec3 centro(0.0f);
    //     for (auto mesh : meshes)
    //     {
    //         // get_position() es del Transform, devuelve vec3
    //         centro += mesh->get_position();
    //     }
    //     centro /= static_cast<float>(meshes.size());

    //     // 4. Definimos un ángulo estático para que persista entre cuadros
    //     static float anguloCam = 0.0f;             // en radianes
    //     const float velocidadAngular = 0.5f;       // radianes por segundo (ajusta a tu gusto)
    //     // O, si prefieres “radianes por frame” fijo en lugar de delta_time:
    //     // const float velocidadXFrame = 0.02f;     // radianes por cuadro, fijo

    //     // 5. Actualizamos ángulo usando delta_time para velocidad constante en segundos
    //     anguloCam += velocidadAngular * delta_time;
    //     // Si usaras “por cuadro” en vez de “por segundo”:
    //     // anguloCam += velocidadXFrame;

    //     // 6. Elegimos un radio y una altura de la órbita
    //     const float radio = 10.0f;   // distancia constante entre cámara y centro
    //     const float altura = 3.0f;   // eleva la cámara sobre el centro en Y

    //     // 7. Calculamos la nueva posición de la cámara en coordenadas polares (XZ)
    //     float camX = centro.x + radio * cos(anguloCam);
    //     float camZ = centro.z + radio * sin(anguloCam);
    //     float camY = centro.y + altura;

    //     // 8. Obtenemos el puntero a la cámara principal
    //     Camera* cam = get_main_camera();
    //     if (!cam)
    //         return; // Si no hay cámara, devolvemos

    //     // 9. Asignamos la posición y el target (mirar siempre al centro)
    //     cam->set_location(glm::vec3(camX, camY, camZ));
    //     cam->set_target(centro);

    //     // 10. Marcamos dirty (aunque set_location y set_target ya lo hacen internamente)
    //     //    para que las matrices se recalculen cuando se necesite.
    // }

    
//     void Scene::update(float delta_time)
//     {
//         static auto meshes = collect_components<Mesh>();

//         static float angle = 0.f;
//         static int frame_count = 0;
//         frame_count++;
//         angle += 0.025f * (1 + sin(angle * 0.1f)); // Varying rotation speed
//         float z_pos = +5.f * sin(frame_count * 0.1f);

//         for (auto mesh : meshes)
//         {
//             mesh->rotate(angle, glm::fvec3(0.f, 1.f, 0.f));
//             mesh->set_position(glm::fvec3(0.f, -1.f, z_pos));
//             mesh->recalculate ();
//         }
//     }
    
    void Scene::task_update (std::stop_token stop_token, float delta_time)
    {    
        while (not running)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); ///< Espera inicial para asegurar que la cámara esté lista.
        }

        auto meshes = collect_components<Mesh>();

        glm::vec3 centro(0.0f);

        float anguloCam = 0.0f;
        const float pasoPorFrame = 0.02f; // radians/frame

        const float radio = 10.0f;
        const float altura = 3.0f;
        
        float angle = 0.f;
        int frame_count = 0;

        Camera * cam = nullptr;

        do
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); ///< Espera inicial para asegurar que la cámara esté lista.
            cam = get_main_camera();
        } while (not cam);
        
        
        while (not stop_token.stop_requested())
        {    
            centro = glm::vec3(0.0f);
            for (auto mesh : meshes)
                centro += mesh->get_position();
            centro /= static_cast<float>(meshes.size());

            anguloCam += pasoPorFrame;
            if (anguloCam > glm::two_pi<float>())
                anguloCam -= glm::two_pi<float>();

            float camX = centro.x + radio * cos(anguloCam);
            float camZ = centro.z + radio * sin(anguloCam);
            float camY = centro.y + altura;

            cam->set_location(glm::vec3(camX, camY, camZ));
            cam->set_target(centro);

#ifdef CONFIG_GRAPHICS_PARALLEL_ENABLED
            thread_pool.sem_mesh_ready.release();

            thread_pool.sem_render_done.acquire();
#endif
            
            meshes = collect_components<Mesh>();

            // std::this_thread::sleep_for(std::chrono::milliseconds(1)); ///< Espera para evitar saturar el CPU
        }
    }
    
    
    // Explicit template instantiations for common component types
    template std::vector< std::shared_ptr < Mesh >   > Scene::collect_components< Mesh   >();
    template std::vector< std::shared_ptr < Camera > > Scene::collect_components< Camera >();
}
