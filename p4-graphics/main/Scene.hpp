//
//  Scene.hpp
//  FileToOBJ
//
//  Created by Andrés Ragot on 6/4/25.
//

#pragma once

#include "Node.hpp"
#include "Camera.hpp"
#include <string>
#include <unordered_map>
#include <typeindex>
#include <functional>
#include <memory>
#include "Id.hpp"
#include <stop_token>

namespace Ragot
{
    class Scene
    {
    private:
        Camera * main_camera = nullptr;
        std::shared_ptr < Node > root_node;
        std::unordered_map<basics::Id, std::shared_ptr < Node > > named_nodes;
        std::atomic<bool> running = false;
        
    public:
        Scene();
       ~Scene() = default;
        Scene (Camera * camera);
        
        // Node management
        void add_node(std::shared_ptr < Node > node, const basics::Id name);
        void remove_node(std::shared_ptr < Node > node);
        std::shared_ptr < Node > find_node(const basics::Id name);
        
        // Camera management
        void set_main_camera(Camera * camera);
        Camera * get_main_camera() const { return main_camera; }
        
        // Scene traversal
        void traverse(const std::function<void(std::shared_ptr < Node >) >& callback);
        
        // Collect renderables
        template<typename T>
        std::vector<std::shared_ptr < T > > collect_components();
        
        // Scene lifecycle
        void update(float delta_time);
        
        // Root node access
              std::shared_ptr < Node > get_root()       { return root_node; }
        const std::shared_ptr < Node > get_root() const { return root_node; }

        void start() { running = true; }
        void stop() { running = false; }
        
    private:
        void task_update (std::stop_token, float delta_time);
    };
}
