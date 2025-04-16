//
//  Node.hpp
//  FileToOBJ
//
//  Created by Andrés Ragot on 1/4/25.
//

#pragma once
#include "CommonTypes.hpp"
#include <vector>
#include "Transform.hpp"
#include <algorithm>

namespace Ragot
{
    using glm::mat4;
    
    class Node : public Transform
    {
    
    protected:
        std::vector < Node * > children;
        Node * parent = nullptr;
        
    public:
        Node () = default;
        virtual ~Node () = default;
        
        Node (const Node &) = delete;
        Node (const Node &&) = delete;
        Node & operator = (const Node &) = delete;
        Node & operator = (const Node &&) = delete;
    public:
        void add_child (Node * child)
        {
            if (child)
            {
                children.emplace_back(child);
                child->parent = this;
            }
        }
        
        void remove_child (Node * child)
        {
            if (child)
            {
                auto it = std::remove(children.begin(), children.end(), child);
                if (it != children.end())
                {
                    children.erase(it, children.end());
                    child->parent = nullptr;
                }
            }
        }
        
        const std::vector<Node*>& get_children() const { return children; }
        
        mat4 get_transform_matrix() override
        {
            mat4 transform_matrix = Transform::get_transform_matrix();
            if (parent)
                transform_matrix = parent->get_transform_matrix() * transform_matrix;
                
            return transform_matrix;
        }
        
    };
}
