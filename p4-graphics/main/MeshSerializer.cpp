//
//  MeshSerializer.cpp
//  FileToOBJ
//
//  Created by Andrés Ragot on 10/3/25.
//

#include <iostream>
#include <fstream>
#include "MeshSerializer.hpp"

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
            std::cerr << "Error al abrir el archivo: " << path << std::endl;
            return false;
        }
        
        return true;
    }
}
