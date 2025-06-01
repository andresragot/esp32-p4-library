/**
 * @file Components.hpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief This file defines the Component class, which serves as a base class for components in the Ragot engine.
 * @details The Component class inherits from Node and provides functionality to manage a collection of components.
 * It allows adding and removing components, and provides access to the list of components.
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

#pragma once

#include "Node.hpp"
#include <vector>
#include <algorithm>


namespace Ragot
{
    /**
     * @class Component
     * @brief Base class for components in the Ragot engine.
     * 
     * The Component class serves as a base class for all components in the Ragot engine.
     * It allows for the management of a collection of components, providing methods to add and remove components,
     * and access the list of components.
     */
    class Component : public Node
    {
    public:

        /**
         * @brief Default constructor for the Component class.
         * 
         * Initializes an empty component with no parent.
         */
        Component() = default;

        /**
         * @brief Default virtual destructor for the Component class.
         * 
         * Cleans up the component and its resources.
         */
        virtual ~Component() = default;
        
        /**
         * @brief Deleted copy constructor for the Component class.
         */
        Component(const Component & ) = delete;

        /**
         * @brief Deleted move constructor for the Component class.
         * 
         * Prevents moving a Component instance.
         */
        Component(const Component &&) = delete;

        /**
         * @brief Deleted assignment operator for the Component class.
         * 
         * Prevents assignment of a Component instance.
         */
        Component & operator = (const Component & ) = delete;

        /**
         * @brief Deleted move assignment operator for the Component class.
         * 
         * Prevents moving a Component instance.
         */
        Component & operator = (const Component &&) = delete;
        
    protected:

        /**
         * @brief Collection of components managed by this Component instance.
         * 
         * This vector holds shared pointers to the components that are part of this Component instance.
         */
        std::vector < std::shared_ptr < Component > > components;
        
    public:

        /**
         * @brief Adds a component to the collection.
         * 
         * This method adds a shared pointer to a component to the collection and sets its parent to this Component instance.
         * 
         * @param component Shared pointer to the component to be added.
         */
        void add_component(std::shared_ptr < Component > component)
        {
            if (component)
            {
                components.emplace_back(component);
                component->parent = this;
            }
        }
        
        /**
         * @brief Removes a component from the collection.
         * 
         * This method removes a shared pointer to a component from the collection and sets its parent to nullptr.
         * 
         * @param component Shared pointer to the component to be removed.
         */
        void remove_component (std::shared_ptr < Component > component)
        {
            if (component)
            {
                auto it = std::remove(components.begin(), components.end(), component);
                if (it != components.end())
                {
                    components.erase(it, components.end());
                    component->parent = nullptr;
                }
            }
        }

        /**
         * @brief Gets the collection of components.
         * 
         * This method returns a constant reference to the vector of components managed by this Component instance.
         * 
         * @return const std::vector<std::shared_ptr<Component>>& Reference to the vector of components.
         */
        const std::vector<std::shared_ptr < Component > > get_components() const { return components; }
        
    };
}
