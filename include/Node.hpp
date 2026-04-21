/**
 * @file Node.hpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief This file implements the Node class, which represents a node in a scene graph for 3D rendering.
 * @details The Node class is a part of the Ragot engine and extends the Transform class to include child nodes.
 * It allows for hierarchical transformations and management of child nodes, enabling complex scene structures.
 * The class provides methods to add and remove child nodes, retrieve the list of children, and compute the transformation matrix.
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
 * 
 */

#pragma once
#include "CommonTypes.hpp"
#include <vector>
#include "Transform.hpp"
#include <algorithm>
#include <memory>

namespace Ragot
{
    using glm::mat4;
    
    /**
     * @class Node
     * @brief Represents a node in a scene graph for 3D rendering.
     * 
     * The Node class extends the Transform class to include child nodes,
     * allowing for hierarchical transformations and management of child nodes.
     */
    class Node : public Transform
    {
    
    protected:

        std::vector < std::shared_ptr < Node > > children; ///< List of child nodes
        Node * parent = nullptr; ///< Pointer to the parent node
        
    public:
        /**
         * @brief Default constructor for Node.
         * Initializes an empty node with no parent and no children.
         */
        Node () = default;

        /**
         * @brief Default destructor for Node.
         * Cleans up the node and its children.
         */
        virtual ~Node () = default;
        
        /**
         * @brief Deleted copy constructor for Node.
         * Prevents copying of Node instances.
         */
        Node (const Node &) = delete;

        /**
         * @brief Deleted move constructor for Node.
         * Prevents moving of Node instances.
         */
        Node (const Node &&) = delete;

        /**
         * @brief Deleted assignment operator for Node.
         * Prevents assignment of Node instances.
         */
        Node & operator = (const Node &) = delete;

        /**
         * @brief Deleted move assignment operator for Node.
         * Prevents moving of Node instances.
         * 
         * @return Node& Reference to the current object.
         */
        Node & operator = (const Node &&) = delete;
    public:

        /**
         * @brief Get the parent node.
         * 
         * @return Node* Pointer to the parent node, or nullptr if no parent exists.
         */
        void add_child (std::shared_ptr < Node > child)
        {
            if (child)
            {
                children.emplace_back(child);
                child->parent = this;
                child->dirty = true;
                dirty = true;
            }
        }
        
        /**
         * @brief Remove a child node.
         * 
         * This method removes the specified child node from the list of children.
         * If the child exists, it is removed and its parent pointer is set to nullptr.
         * 
         * @param child The child node to remove.
         */
        void remove_child (std::shared_ptr < Node > child)
        {
            if (child)
            {
                auto it = std::remove(children.begin(), children.end(), child);
                if (it != children.end())
                {
                    children.erase(it, children.end());
                    child->parent = nullptr;
                    child->dirty = true;
                }
            }
        }
        
        /**
         * @brief Get the parent node.
         * 
         * @return Node* Pointer to the parent node, or nullptr if no parent exists.
         */
        const std::vector< std::shared_ptr < Node > >& get_children() const { return children; }
        
        /**
         * @brief Get the transform matrix object.
         * 
         * @return mat4 The transformation matrix of the node, including its parent's transformation.
         */
        mat4 get_transform_matrix() override
        {
            mat4 transform_matrix = Transform::get_transform_matrix();
            if (parent)
                transform_matrix = parent->get_transform_matrix() * transform_matrix;
            
            return transform_matrix;
        }
        
    };
}
