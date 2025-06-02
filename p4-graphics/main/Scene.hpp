/**
 * @file Scene.hpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief Implementation of the Scene class for managing 3D scenes.
 * @details The Scene class provides methods to manage nodes, cameras, and scene traversal.
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
#include "Camera.hpp"
#include <string>
#include <unordered_map>
#include <typeindex>
#include <functional>
#include <memory>
#include "Id.hpp"
#include <stop_token>

namespace Ragot
{
    /**
     * @class Scene
     * @brief Class for managing a 3D scene.
     * 
     * The Scene class provides methods to manage nodes, cameras, and scene traversal.
     * It allows adding and removing nodes, setting the main camera, and traversing the scene graph.
     */
    class Scene
    {
    private:
        Camera * main_camera = nullptr; ///< Pointer to the main camera used for rendering the scene.
        std::shared_ptr < Node > root_node; ///< Shared pointer to the root node of the scene graph.
        std::unordered_map<basics::Id, std::shared_ptr < Node > > named_nodes; ///< Map of named nodes for quick access by name.
        std::atomic<bool> running = false; ///< Flag indicating whether the scene is currently running or not.
        
    public:
        /**
         * @brief Default constructor for the Scene class.
         * 
         * Initializes the scene with a root node and prepares it for use.
         */
        Scene();
        
        /**
         * @brief Constructs a Scene object with a specified camera.
         * 
         * Initializes the scene with a root node and sets the main camera.
         * 
         * @param camera Pointer to the Camera object to be used as the main camera for the scene.
         */
       ~Scene() = default;

        /**
         * @brief Construct a new Scene object
         * 
         * @param camera Pointer to the Camera object to be used as the main camera for the scene.
         */
        Scene (Camera * camera);
        
        /**
         * @brief Adds a node to the scene with a specified name.
         * 
         * @param node Shared pointer to the Node object to be added to the scene.
         * @param name Unique identifier for the node, used for quick access.
         */
        void add_node(std::shared_ptr < Node > node, const basics::Id name);

        /**
         * @brief Removes a node from the scene.
         * 
         * @param node Shared pointer to the Node object to be removed from the scene.
         */
        void remove_node(std::shared_ptr < Node > node);

        /**
         * @brief Finds a node in the scene by its name.
         * 
         * @param name Unique identifier for the node to be found.
         * @return std::shared_ptr<Node> Shared pointer to the found Node object, or nullptr if not found.
         */
        std::shared_ptr < Node > find_node(const basics::Id name);
        
        /**
         * @brief Sets the main camera for the scene.
         * 
         * @param camera Pointer to the Camera object to be set as the main camera for the scene.
         */
        void set_main_camera(Camera * camera);

        /**
         * @brief Gets the main camera of the scene.
         * 
         * @return Camera* Pointer to the main Camera object used for rendering the scene.
         */
        Camera * get_main_camera() const { return main_camera; }
        
        /**
         * @brief Traverses the scene graph and applies a callback function to each node.
         * 
         * This method allows for custom operations on each node in the scene graph.
         * 
         * @param callback Function to be called for each node in the scene graph.
         */
        void traverse(const std::function<void(std::shared_ptr < Node >) >& callback);
        
        /**
         * @brief Collects all components of a specified type from the scene.
         * 
         * This method traverses the scene graph and collects all components of the specified type.
         * 
         * @tparam T The type of component to collect.
         * @return std::vector<std::shared_ptr<T>> A vector containing shared pointers to the collected components.
         */
        template<typename T>
        std::vector<std::shared_ptr < T > > collect_components();
        
        /**
         * @brief Updates the scene with a specified delta time.
         * 
         * This method updates the scene, allowing for animations or other time-based changes.
         * 
         * @param delta_time The time elapsed since the last update, in seconds.
         */
        void update(float delta_time);
        
        /**
         * @brief Get the root object
         * 
         * @return std::shared_ptr < Node > 
         */
              std::shared_ptr < Node > get_root()       { return root_node; }
        /**
         * @brief Get the root object
         * 
         * @return const std::shared_ptr < Node > 
         */
        const std::shared_ptr < Node > get_root() const { return root_node; }

        /**
         * @brief Starts the scene, setting the running flag to true.
         * 
         * This method is used to indicate that the scene is active and should be updated.
         */
        void start() { running = true; }

        /**
         * @brief Stops the scene, setting the running flag to false.
         * 
         * This method is used to indicate that the scene is no longer active and should not be updated.
         */
        void stop() { running = false; }
        
    private:
        /**
         * @brief Task to update the scene in a separate thread.
         * 
         * This method runs in a separate thread and updates the scene based on the delta time.
         * It uses a stop token to allow for graceful termination of the task.
         * 
         * @param stop_token Token to signal when the task should stop.
         * @param delta_time The time elapsed since the last update, in seconds.
         */
        void task_update (std::stop_token, float delta_time);
    };
}
