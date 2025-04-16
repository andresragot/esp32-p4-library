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

namespace Ragot
{
    class Scene
    {
    private:
        Camera * main_camera = nullptr;
        Node root_node;
        std::unordered_map<basics::Id, Node*> named_nodes;
        
    public:
        Scene() = default;
       ~Scene() = default;
        Scene (Camera * camera) : main_camera (camera) {}
        
        // Node management
        void add_node(Node* node, const basics::Id name);
        void remove_node(Node* node);
        Node * find_node(const basics::Id name);
        
        // Camera management
        void set_main_camera(Camera * camera);
        Camera * get_main_camera() const { return main_camera; }
        
        // Scene traversal
        void traverse(const std::function<void(Node*)>& callback);
        
        // Collect renderables
        template<typename T>
        std::vector<T*> collect_components();
        
        // Scene lifecycle
        void update(float delta_time);
        
        // Root node access
              Node * get_root()       { return &root_node; }
        const Node * get_root() const { return &root_node; }
    };
}
