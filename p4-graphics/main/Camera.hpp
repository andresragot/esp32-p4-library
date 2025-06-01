/**
 * @file Camera.hpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief This file implements the Camera class, which manages camera properties and operations in a 3D space.
 * @details The Camera class provides functionality to log camera information, calculate normals for faces, and determine visibility of faces based on their orientation relative to the camera.
 * It uses GLM (OpenGL Mathematics) for vector and matrix operations, and includes methods for calculating normals and checking face visibility.
 * @version 0.1
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

#include "CommonTypes.hpp"
#include "Components.hpp"
#include "Transform.hpp"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

namespace Ragot
{   
    /**
     * @class Camera
     * @brief Represents a camera in a 3D space, managing its properties and transformations.
     * 
     * The Camera class provides functionality to set and get camera properties such as field of view, near and far planes,
     * aspect ratio, and target position. It also computes projection, view, and combined view-projection matrices,
     * and provides methods for projecting world coordinates to normalized device coordinates (NDC).
     */
    class Camera : public Component
    {
        /**
         * @brief Tag for logging camera-related messages.
         */
        static const char* CAMERA_TAG;

    public:
        using Matrix4x4 = glm::mat4;

    private:
        float fov;              ///< vertical field of view in degrees
        float near_plane;       ///< near clipping plane distance
        float far_plane;        ///< far clipping plane distance
        float aspect_ratio;     ///< aspect ratio (width/height)

        glm::vec3 target;       ///< world space look-at target

        mutable Matrix4x4 projection_matrix;    ///< cached projection matrix
        mutable Matrix4x4 view_matrix;          ///< cached view matrix
        mutable Matrix4x4 vp_matrix;            ///< cached view-projection matrix
        mutable bool projDirty = true;          ///< flag to indicate if projection matrix is dirty
        mutable bool viewDirty = true;          ///< flag to indicate if view matrix is dirty
        mutable bool vpDirty   = true;          ///< flag to indicate if view-projection matrix is dirty

    public:
        /**
         * @brief Delete default constructor to prevent instantiation without parameters.
         */
        Camera() = delete;
        /**
         * @brief Constructs a Camera object with specified parameters.
         * 
         * @param aspect_ratio Aspect ratio of the camera (default is 1.0).
         * @param near_plane Distance to the near clipping plane (default is 1.0).
         * @param far_plane Distance to the far clipping plane (default is 100.0).
         * @param fov_deg Vertical field of view in degrees (default is 60.0).
         */
        Camera(float aspect_ratio = 1.f,
               float near_plane   = 1.f,
               float far_plane    = 100.f,
               float fov_deg      = 60.f)
            : fov(fov_deg), near_plane(near_plane), far_plane(far_plane), aspect_ratio(aspect_ratio)
        {
            set_position(glm::vec3(0.f));
            target = glm::vec3(0.f, 0.f, -1.f);
        }
        /**
         * @brief Default destructor for Camera class.
         */
        ~Camera() = default;

        // --- Getters ---
        /**
         * @brief Returns the camera's field of view in degrees.
         * 
         * @return float The field of view in degrees.
         */
        float get_fov() const           { return fov; }

        /**
         * @brief Returns the camera's near clipping plane distance.
         * 
         * @return float The distance to the near clipping plane.
         */
        float get_near_plane() const    { return near_plane; }

        /**
         * @brief Returns the camera's far clipping plane distance.
         * 
         * @return float The distance to the far clipping plane.
         */
        float get_far_plane() const     { return far_plane; }

        /**
         * @brief Returns the camera's aspect ratio (width/height).
         * 
         * @return float The aspect ratio of the camera.
         */
        float get_aspect_ratio() const  { return aspect_ratio; }

        /**
         * @brief Returns the camera's position in world space.
         * 
         * @return glm::vec3 The position of the camera.
         */
        glm::vec3 get_location() const  { return get_position(); }

        /**
         * @brief Returns the camera's target position in world space.
         * 
         * @return glm::vec3 The target position of the camera.
         */
        glm::vec3 get_target() const    { return target; }

        /**
         * @brief Returns the camera's position in world space.
         * 
         * @return glm::vec3 The position of the camera.
         */
        bool is_dirty() const { return projDirty || viewDirty || vpDirty; }

        // --- Setters (mark dirty) ---

        /**
         * @brief Set the fov object
         * 
         * @param deg Field of view in degrees.
         */
        void set_fov(float deg)            { fov = deg; projDirty = true; vpDirty = true; }

        /**
         * @brief Set the near clipping plane distance.
         * 
         * @param np Distance to the near clipping plane.
         */
        void set_near_plane(float np)      { near_plane = np; projDirty = true; vpDirty = true; }

        /**
         * @brief Set the far clipping plane distance.
         * 
         * @param fp Distance to the far clipping plane.
         */
        void set_far_plane(float fp)       { far_plane = fp; projDirty = true; vpDirty = true; }

        /**
         * @brief Set the aspect ratio of the camera.
         * 
         * @param ar Aspect ratio (width/height).
         */
        void set_aspect_ratio(float ar)    { aspect_ratio = ar; projDirty = true; vpDirty = true; }

        /**
         * @brief Set the camera's position in world space.
         * 
         * @param p Position of the camera.
         */
        void set_location(const glm::vec3 &p) { set_position(p); viewDirty = true; vpDirty = true; }

        /**
         * @brief Set the camera's target position in world space.
         * 
         * @param t Target position of the camera.
         */
        void set_target(const glm::vec3 &t)   { target = t; viewDirty = true; vpDirty = true; }

        /**
         * @brief Get the projection matrix object
         * 
         * @return const Matrix4x4& 
         */
        const Matrix4x4& get_projection_matrix() const
        {
            if (projDirty)
            {
                projection_matrix = glm::perspective(glm::radians(fov), aspect_ratio, near_plane, far_plane);
                projDirty = false;
            }
            return projection_matrix;
        }

        /**
         * @brief Get the view matrix object
         * 
         * @return const Matrix4x4& 
         */
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

        /**
         * @brief Get the vp matrix object
         * 
         * @return const Matrix4x4& 
         */
        const Matrix4x4& get_vp_matrix() const
        {
            if (vpDirty)
            {
                vp_matrix = get_projection_matrix() * get_view_matrix();
                vpDirty = false;
            }
            return vp_matrix;
        }

        /**
         * @brief Get the view direction object
         * 
         * @return glm::vec3 
         */
        glm::vec3 get_view_direction() const
        {
            return glm::normalize(target - get_position());
        }

        /**
         * @brief Get the right direction object
         * 
         * @return glm::vec3 
         */
        glm::vec3 get_right_direction() const
        {
            return glm::normalize(glm::cross(get_view_direction(), glm::vec3(0.f,1.f,0.f)));
        }

        /**
         * @brief Get the up direction object
         * 
         * @return glm::vec3 
         */
        glm::vec3 get_up_direction() const
        {
            return glm::normalize(glm::cross(get_right_direction(), get_view_direction()));
        }

        /**
         * @brief 
         * 
         * @param worldPos 
         * @return glm::vec3 
         */
        glm::vec3 project_to_ndc(const glm::vec4 &worldPos) const
        {
            glm::vec4 clip = get_vp_matrix() * worldPos;
            return (clip.w == 0.f) ? glm::vec3(0.f) : glm::vec3(clip) / clip.w;
        }

        /**
         * @brief Calculate the normal vector of a face defined by three vertices.
         * 
         * @param v1 First vertex of the face.
         * @param v2 Second vertex of the face.
         * @param v3 Third vertex of the face.
         * @return vertex_t 
         */
        vertex_t calculate_normal(const vertex_t &v1, const vertex_t &v2, const vertex_t &v3);

        /**
         * @brief Check if a face defined by three vertices is visible from the camera's perspective.
         * 
         * A face is considered visible if its normal vector points towards the camera.
         * 
         * @param v1 First vertex of the face.
         * @param v2 Second vertex of the face.
         * @param v3 Third vertex of the face.
         * @return true if the face is visible, false otherwise.
         */
        bool is_face_visible(const vertex_t &v1, const vertex_t &v2, const vertex_t &v3);

        /**
         * @brief Logs the camera's properties for debugging purposes.
         * 
         * This method logs the camera's position, target, field of view, near and far planes, and aspect ratio.
         */
        void log_camera_info() const;

    };
}
