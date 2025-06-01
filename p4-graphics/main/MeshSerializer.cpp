/**
 * @file MeshSerializer.cpp
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

#include <iostream>
#include <fstream>
#include <cerrno>
#include "MeshSerializer.hpp"
#include <filesystem>

namespace Ragot
{
    MeshSerializer & serializer = MeshSerializer::instance();
    
    bool MeshSerializer::save_to_obj(const Mesh & mesh, const std::filesystem::path &path)
    {
        std::ofstream file (path, std::ofstream::binary);
        
        if (file.good() && file.is_open())
        {
            for (const auto & vertex : mesh.get_vertices())
            {
                file << "v " << vertex.x << " " << vertex.y << " " << vertex.z << std::endl;
            }
            
            for (const auto & face : mesh.get_faces())
            {
                if (face.is_quad)
                {
                    file << "f " << face.v1 + 1 << " " << face.v2 + 1 << " " << face.v3 + 1 << " " << face.v4 + 1 << std::endl;
                }
                else
                {
                    file << "f " << face.v1 + 1 << " " << face.v2 + 1 << " " << face.v3 + 1 << std::endl;
                }
            }
        }
        else
        {
            // std::cerr << "Error al abrir el archivo: " << path << std::endl;
            int err = errno;
            std::cerr << "Error al abrir el archivo '" << path
                      << "': " << std::strerror(err)
                      << " (errno=" << err << ")\n";
            std::cout << "Ruta completa: " << std::filesystem::absolute(path) << std::endl;
            return false;
        }
        
        return true;
    }
}
