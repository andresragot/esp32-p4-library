//
//  Scene.cpp
//  FileToOBJ
//
//  Created by Andrés Ragot on 6/4/25.
//

#include "Scene.hpp"
#include "Mesh.hpp"
#include <queue>
#include <algorithm>
#include "Thread_Pool.hpp"
#include "Logger.hpp"

namespace Ragot
{
    Scene::Scene ()
    {
        std::cout << "Scene constructor" << std::endl;
        thread_pool.submit_with_stop (std::bind (&Scene::task_update, this, std::placeholders::_1, 0.f));
    }
    
    Scene::Scene (Camera * camera) : main_camera (camera)
    {
        std::cout << "Scene constructor with camera" << std::endl;
        thread_pool.submit_with_stop (std::bind (&Scene::task_update, this, std::placeholders::_1, 0.f));
    }

    void Scene::add_node(Node* node, const basics::Id name)
    {
        if (!node) return;
        
        root_node.add_child(node);
        
        named_nodes[name] = node;
    }
    
    void Scene::remove_node(Node* node)
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
        
        root_node.remove_child(node);
    }
    
    Node* Scene::find_node(const basics::Id name)
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
    
    void Scene::traverse(const std::function < void (Node*) > & callback)
    {
        if (!callback) return;
        
        std::queue < Node* > node_queue;
        node_queue.push (&root_node);
        
        while (!node_queue.empty()) 
        {
            Node * current = node_queue.front();
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
    std::vector<T*> Scene::collect_components()
    {
        ///< @todo sacar este vector.
        std::vector< T *> result;
        
        traverse([&result](Node* node) 
        {
            // Check if node is a Component
            Component * component = dynamic_cast< Component *>(node);
            if (component) 
            {
                // Try to cast to the requested type
                T * typed_component = dynamic_cast< T *>(component);
                if (typed_component) 
                {
                    result.emplace_back(typed_component);
                }
                
                // Also check components attached to this component
                for (auto* subComponent : component->get_components()) 
                {
                    T * typed_subComponent = dynamic_cast< T *>(subComponent);
                    if (typed_subComponent) 
                    {
                        result.emplace_back(typed_subComponent);
                    }
                }
            }
        });
        
        return result;
    }
    
    void Scene::update(float delta_time)
    {
        static auto meshes = collect_components<Mesh>();

        static float angle = 0.f;
        static int frame_count = 0;
        frame_count++;
        angle += 0.025f * (1 + sin(angle * 0.1f)); // Varying rotation speed
        float z_pos = +5.f * sin(frame_count * 0.1f);

        for (auto mesh : meshes)
        {
            mesh->rotate(angle, glm::fvec3(0.f, 1.f, 0.f));
            mesh->set_position(glm::fvec3(0.f, -1.f, z_pos));
            mesh->recalculate ();
        }
    }
    
    void Scene::task_update (std::stop_token stop_token, float delta_time)
    {    
        logger.Log ("SCENE", 1, "=== Iniciando task_update() ===");

        auto meshes = collect_components<Mesh>();
        
        float angle = 0.f;
        int frame_count = 0;
        
        angle += 0.025f * (1 + sin(angle * 0.1f)); // Varying rotation speed
        float z_pos;
        
        float time = 1 / 60 * 1000; ///< 60 fps
        long long int time_ms = static_cast<long long int>(time);
        
        while (not stop_token.stop_requested())
        {
            z_pos = +5.f * sin(frame_count * 0.1f);
            frame_count ++;
            
            for (auto mesh : meshes)
            {
                mesh->rotate(angle, glm::fvec3(0.f, 1.f, 0.f));
                mesh->set_position(glm::fvec3(0.f, -1.f, z_pos));
                mesh->recalculate ();
            }
            
            thread_pool.sem_mesh_ready.release();

            thread_pool.sem_render_done.acquire();
            
            meshes = collect_components<Mesh>();
            // std::this_thread::sleep_for(std::chrono::milliseconds(time_ms)); ///< 60 fps.
        }
    }
    
    
    // Explicit template instantiations for common component types
    template std::vector< Mesh   *> Scene::collect_components< Mesh   >();
    template std::vector< Camera *> Scene::collect_components< Camera >();
}
