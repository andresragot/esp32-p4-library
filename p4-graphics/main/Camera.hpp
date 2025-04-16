//
//  Camera.hpp
//  FileToOBJ
//
//  Created by Andrés Ragot on 11/2/25.
//

#pragma once

#include "CommonTypes.hpp"
#include "Components.hpp"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

namespace Ragot
{    
    class Camera : public Component
    {
    public:
        using Matrix4x4 = glm::mat4;
        using Point = glm::vec4;
    
    private:
        float fov;
        float near_plane;
        float far_plane;
        float aspect_ratio;
        
        Point location;
        Point target;
        
        Matrix4x4 projection_matrix;
    public:
        Camera () = delete;
        Camera (float aspect_ratio = 1.f, float near_plane = 0.1f, float far_plane = 1000.f, float fov = 60.f) { reset (fov, near_plane, far_plane, aspect_ratio); }
       ~Camera () = default;
       
        float get_fov () const { return fov; }
        float get_near_plane () const { return near_plane; }
        float get_far_plane () const { return far_plane; }
        float get_aspect_ratio () const { return aspect_ratio; }
        void set_fov (float fov) { this->fov = fov; }
        void set_near_plane (float near_plane) { this->near_plane = near_plane; }
        void set_far_plane (float far_plane) { this->far_plane = far_plane; }
        void set_aspect_ratio (float aspect_ratio) { this->aspect_ratio = aspect_ratio; }
        
        const Point & get_location () const { return location; }
        const Point & get_target () const { return target; }
        void set_location (const Point & location) { this->location = location; }
        void set_target (const Point & target) { this->target = target; }
            
        const Matrix4x4 & get_projection_matrix () const { return projection_matrix; }
        
        void reset (float new_fov, float new_near_plane, float new_far_plane, float new_aspect_ratio)
        {
            set_fov(new_fov);
            set_near_plane(new_near_plane);
            set_far_plane(new_far_plane);
            set_aspect_ratio(new_aspect_ratio);
            set_location({0.f, 0.f, 0.f, 1.f});
            set_target({0.f, 0.f, -1.f, 1.f});
            calculate_projection_matrix();
        }
        
        Matrix4x4 get_view_matrix () const
        {
            return glm::lookAt(
                glm::vec3(location.x, location.y, location.z),
                glm::vec3(target.x, target.y, target.z),
                glm::vec3(0.f, 1.f, 0.f)
            );
        }
       
       vertex_t calculate_normal (const vertex_t & v1, const vertex_t & v2, const vertex_t & v3);
       bool is_face_visible (const vertex_t & v1, const vertex_t & v2, const vertex_t & v3);
       
    private:
        void normalize_direction();
        void calculate_projection_matrix()
        {
            projection_matrix = glm::perspective(glm::radians(fov), aspect_ratio, near_plane, far_plane);
        }
    };
}


