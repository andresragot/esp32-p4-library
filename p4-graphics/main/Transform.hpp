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
        
    public:
        Transform () : position (0.f), rotation (0.f), scale (1.f) {}
        virtual ~Transform () = default;
        
    public:
        void set_position (const vec3 & pos) { position = pos; }
        vec3 get_position () const { return position; }
        
        void set_rotation (const vec3 & rot) { rotation = rot; }
        vec3 get_rotation () const { return rotation; }
        
        void set_scale (const vec3 & scale) {this->scale = scale; }
        vec3 get_scale () const { return scale; }
        
    public:
        virtual mat4 get_transform_matrix ()
        {
            mat4 transform_matrix(1);
            transform_matrix = glm::translate(transform_matrix, position);
            transform_matrix = glm::scale (transform_matrix, scale);
            
            glm::quat quaternion_rotation = glm::quat (glm::radians (rotation));
            transform_matrix *= glm::mat4_cast (quaternion_rotation);
            
            return transform_matrix;
        }
    };
}
