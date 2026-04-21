/**
 * @file Transform.hpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief Implementation of the Transform class for 3D transformations.
 * @details The Transform class provides methods to manage position, rotation, and scale of 3D objects.
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

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>         // translate, rotate, scale, perspective
#include <gtc/type_ptr.hpp>                 // value_ptr, quat

namespace Ragot
{
    using glm::vec3;
    using glm::mat4;

    /**
     * @class Transform
     * @brief A class representing a 3D transformation with position, rotation, and scale.
     * 
     * This class provides methods to set and get the position, rotation, and scale of a 3D object.
     * It also provides a method to get the transformation matrix that combines these transformations.
     */
    class Transform
    {
    protected:
        vec3 position; ///< The position of the object in 3D space.
        vec3 rotation; ///< The rotation of the object in degrees around each axis (x, y, z).
        vec3 scale; ///< The scale of the object in 3D space, default is (1, 1, 1).
        bool dirty = true; ///< Flag indicating whether the transformation matrix needs to be recalculated.

    private:
        mat4 transform_matrix; ///< The transformation matrix that combines position, rotation, and scale.
        
    public:
        /**
         * @brief Default constructor for the Transform class.
         * 
         * Initializes position to (0, 0, 0), rotation to (0, 0, 0), and scale to (1, 1, 1).
         */
        Transform () : position (0.f), rotation (0.f), scale (1.f) {}

        /**
         * @brief Virtual destructor for the Transform class.
         * 
         * Ensures proper cleanup of derived classes.
         */
        virtual ~Transform () = default;
        
    public:
        /**
         * @brief Sets the position of the object.
         * 
         * @param pos The new position as a vec3.
         */
        void set_position (const vec3 & pos) 
        { 
            position = pos; 
            dirty = true; 
        }

        /**
         * @brief Gets the current position of the object.
         * 
         * @return vec3 The current position.
         */
        vec3 get_position () const { return position; }
        
        /**
         * @brief Moves the object by a specified vector.
         * 
         * @param offset The vector by which to move the object.
         */
        void set_rotation (const vec3 & rot) 
        {
            rotation = rot; 
            dirty = true; 
        }

        /**
         * @brief Gets the current rotation of the object.
         * 
         * @return vec3 The current rotation in degrees around each axis (x, y, z).
         */
        vec3 get_rotation () const { return rotation; }

        /**
         * @brief Rotates the object by a specified angle around a given axis.
         * 
         * @param angle The angle in degrees to rotate.
         * @param axis The axis around which to rotate, as a vec3.
         */
        void rotate (const float angle, const vec3 & axis)
        {
            rotation += angle * axis;
            if (rotation.x > 360.f) rotation.x -= 360.f;
            dirty = true;
        }
        
        /**
         * @brief Sets the scale of the object.
         * 
         * @param scale The new scale as a vec3.
         */
        void set_scale (const vec3 & scale) 
        {
            this->scale = scale; 
            dirty = true;
        }

        /**
         * @brief Sets the scale of the object uniformly.
         * 
         * @param scale The new uniform scale factor.
         */
        vec3 get_scale () const { return scale; }

        /**
         * @brief Checks if the transformation matrix is dirty (needs recalculation).
         * 
         * @return true if the transformation matrix is dirty, false otherwise.
         */
        bool is_dirty () const { return dirty; }
        
    public:

        /**
         * @brief Gets the transformation matrix that combines position, rotation, and scale.
         * 
         * This method recalculates the transformation matrix if it is dirty.
         * 
         * @return mat4 The transformation matrix.
         */
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
