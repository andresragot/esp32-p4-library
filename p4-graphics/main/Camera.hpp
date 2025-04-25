#pragma once

#include "CommonTypes.hpp"
#include "Components.hpp"
#include "Transform.hpp"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

namespace Ragot
{
    class Camera : public Component
    {
        static const char* CAMERA_TAG;

    public:
        using Matrix4x4 = glm::mat4;

    private:
        float fov;             // vertical field of view in degrees
        float near_plane;
        float far_plane;
        float aspect_ratio;

        glm::vec3 target;      // world space look-at target

        mutable Matrix4x4 projection_matrix;
        mutable Matrix4x4 view_matrix;
        mutable Matrix4x4 vp_matrix;
        mutable bool projDirty = true;
        mutable bool viewDirty = true;
        mutable bool vpDirty   = true;

    public:
        Camera() = delete;
        Camera(float aspect_ratio = 1.f,
               float near_plane   = 1.f,
               float far_plane    = 100.f,
               float fov_deg      = 60.f)
            : fov(fov_deg), near_plane(near_plane), far_plane(far_plane), aspect_ratio(aspect_ratio)
        {
            set_position(glm::vec3(0.f));
            target = glm::vec3(0.f, 0.f, -1.f);
        }
        ~Camera() = default;

        // --- Getters ---
        float get_fov() const           { return fov; }
        float get_near_plane() const    { return near_plane; }
        float get_far_plane() const     { return far_plane; }
        float get_aspect_ratio() const  { return aspect_ratio; }
        glm::vec3 get_location() const  { return get_position(); }
        glm::vec3 get_target() const    { return target; }
        bool is_dirty() const { return projDirty || viewDirty || vpDirty; }

        // --- Setters (mark dirty) ---
        void set_fov(float deg)            { fov = deg; projDirty = true; vpDirty = true; }
        void set_near_plane(float np)      { near_plane = np; projDirty = true; vpDirty = true; }
        void set_far_plane(float fp)       { far_plane = fp; projDirty = true; vpDirty = true; }
        void set_aspect_ratio(float ar)    { aspect_ratio = ar; projDirty = true; vpDirty = true; }
        void set_location(const glm::vec3 &p) { set_position(p); viewDirty = true; vpDirty = true; }
        void set_target(const glm::vec3 &t)   { target = t; viewDirty = true; vpDirty = true; }

        // Projection matrix cached
        const Matrix4x4& get_projection_matrix() const
        {
            if (projDirty)
            {
                projection_matrix = glm::perspective(glm::radians(fov), aspect_ratio, near_plane, far_plane);
                projDirty = false;
            }
            return projection_matrix;
        }

        // View matrix cached
        const Matrix4x4& get_view_matrix() const
        {
            if (viewDirty)
            {
                view_matrix = glm::lookAt(
                    get_position(),
                    target,
                    glm::vec3(0.f,1.f,0.f)
                );
                viewDirty = false;
            }
            return view_matrix;
        }

        // Combined VP matrix cached
        const Matrix4x4& get_vp_matrix() const
        {
            if (vpDirty)
            {
                vp_matrix = get_projection_matrix() * get_view_matrix();
                vpDirty = false;
            }
            return vp_matrix;
        }

        /** Returns normalized direction from camera to target */
        glm::vec3 get_view_direction() const
        {
            return glm::normalize(target - get_position());
        }

        /** Camera right axis (world up assumed +Y) */
        glm::vec3 get_right_direction() const
        {
            return glm::normalize(glm::cross(get_view_direction(), glm::vec3(0.f,1.f,0.f)));
        }

        /** Camera up axis recalculated */
        glm::vec3 get_up_direction() const
        {
            return glm::normalize(glm::cross(get_right_direction(), get_view_direction()));
        }

        /** Projects world position to NDC space (-1..1) */
        glm::vec3 project_to_ndc(const glm::vec4 &worldPos) const
        {
            glm::vec4 clip = get_vp_matrix() * worldPos;
            return (clip.w == 0.f) ? glm::vec3(0.f) : glm::vec3(clip) / clip.w;
        }

        // Placeholder stubs to satisfy interface
        vertex_t calculate_normal(const vertex_t &v1, const vertex_t &v2, const vertex_t &v3);
        bool is_face_visible(const vertex_t &v1, const vertex_t &v2, const vertex_t &v3);

        // Método para depuración
        void log_camera_info() const;

    };
}
