//
//  Transform.hpp
//  FileToOBJ
//
//  Created by Andrés Ragot on 6/4/25.
//

#pragma once

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>         // translate, rotate, scale, perspective
#include <gtc/type_ptr.hpp>                 // value_ptr, quat

namespace Ragot
{
    using glm::vec3;
    using glm::mat4;

    class Transform
    {
    protected:
        vec3 position;
        vec3 rotation;
        vec3 scale;
        bool dirty = true;

    private:
        mat4 transform_matrix;
        
    public:
        Transform () : position (0.f), rotation (0.f), scale (1.f) {}
        virtual ~Transform () = default;
        
    public:
        void set_position (const vec3 & pos) 
        { 
            position = pos; 
            dirty = true; 
        }
        vec3 get_position () const { return position; }
        
        void set_rotation (const vec3 & rot) 
        {
            rotation = rot; 
            dirty = true; 
        }
        vec3 get_rotation () const { return rotation; }

        void rotate (const float angle, const vec3 & axis)
        {
            rotation += angle * axis;
            if (rotation.x > 360.f) rotation.x -= 360.f;
            dirty = true;
        }
        
        void set_scale (const vec3 & scale) 
        {
            this->scale = scale; 
            dirty = true;
        }
        vec3 get_scale () const { return scale; }

        bool is_dirty () const { return dirty; }
        
    public:
        virtual mat4 get_transform_matrix ()
        {
            if (dirty)
            {
                dirty = false;
                
                mat4 identity(1);
                identity = glm::translate(identity, position);
                identity = glm::scale (identity, scale);
                
                glm::quat quaternion_rotation = glm::quat (glm::radians (rotation));
                identity *= glm::mat4_cast (quaternion_rotation);

                transform_matrix = identity;
            }
            
            return transform_matrix;
        }
    };
}
