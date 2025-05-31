//
//  Components.hpp
//  FileToOBJ
//
//  Created by Andrés Ragot on 6/4/25.
//

#pragma once

#include "Node.hpp"
#include <vector>
#include <algorithm>


namespace Ragot
{
    class Component : public Node
    {
    public:
        Component() = default;
        virtual ~Component() = default;
        
        Component(const Component & ) = delete;
        Component(const Component &&) = delete;
        Component & operator = (const Component & ) = delete;
        Component & operator = (const Component &&) = delete;
        
    protected:
        std::vector < std::shared_ptr < Component > > components;
        
    public:
        void add_component(std::shared_ptr < Component > component)
        {
            if (component)
            {
                components.emplace_back(component);
                component->parent = this;
            }
        }
        
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

        const std::vector<std::shared_ptr < Component > > get_components() const { return components; }
        
    };
}
