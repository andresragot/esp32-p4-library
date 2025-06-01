/**
 * @file MeshSerializer.hpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief This file implements the MeshSerializer class, which provides methods to serialize a Mesh object to an OBJ file format.
 * @details The MeshSerializer class allows saving a Mesh object to an OBJ file, which is a common format for 3D models.
 * It handles the serialization of vertices and faces, ensuring that the data is written in a format compatible with OBJ files.
 * The class is designed to be used in graphics applications where exporting 3D models is required.
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

#include "Mesh.hpp"
#include <filesystem>

namespace Ragot
{
    // Vamos a hacer un Singleton.
    /**
     * @class MeshSerializer
     * @brief Singleton class to serialize Mesh objects to OBJ file format.
     */
    class MeshSerializer
    {
    public:
        /**
         * @brief Gets the singleton instance of the MeshSerializer class.
         * 
         * This method ensures that only one instance of MeshSerializer exists throughout the application.
         * 
         * @return MeshSerializer& Reference to the singleton instance of MeshSerializer.
         */
        static MeshSerializer & instance()
        {
            static MeshSerializer serializer;
            return serializer;
        }
        
    private:
        
        /**
         * @brief Private constructor to prevent instantiation from outside the class.
         * 
         * This constructor is private to enforce the singleton pattern, ensuring that only one instance of MeshSerializer exists.
         */
        MeshSerializer () = default;

        /**
         * @brief Construct a new Mesh Serializer object (deleted).
         */
        MeshSerializer (const MeshSerializer &)  = delete;

        /**
         * @brief Construct a new Mesh Serializer object (deleted).
         * 
         * This constructor is deleted to prevent moving the MeshSerializer instance.
         */
        MeshSerializer (const MeshSerializer &&) = delete;

        /**
         * @brief Assignment operator for the MeshSerializer class (deleted).
         * 
         * This operator is deleted to prevent assignment of MeshSerializer instances.
         * 
         * @return MeshSerializer& Reference to the current object.
         */
        MeshSerializer & operator = (const MeshSerializer &)  = delete;

        /**
         * @brief Assignment operator for the MeshSerializer class (deleted).
         * 
         * This operator is deleted to prevent moving MeshSerializer instances.
         * 
         * @return MeshSerializer& Reference to the current object.
         */
        MeshSerializer & operator = (const MeshSerializer &&) = delete;
        
    public:

        /**
         * @brief Saves a Mesh object to an OBJ file.
         * 
         * This method serializes the vertices and faces of the Mesh object and writes them to the specified OBJ file path.
         * 
         * @param mesh The Mesh object to serialize.
         * @param path The filesystem path where the OBJ file will be saved.
         * @return true if the serialization was successful, false otherwise.
         */
        bool save_to_obj (const Mesh & mesh, const std::filesystem::path & path);
        
    };
    
    extern MeshSerializer & serializer;
}
