//
//  RevolutionMesh.hpp
//  FileToOBJ
//
//  Created by Andrés Ragot on 17/3/25.
//

#pragma once

#include "Mesh.hpp"
#include <iostream>

namespace Ragot
{
    class RevolutionMesh : public Mesh
    {
    public:
        RevolutionMesh (mesh_info_t & mesh_info) : Mesh (mesh_info)
        {
            vertices.reserve (mesh_info.coordinates.size() * (slices + 1));
               faces.reserve (mesh_info.coordinates.size() *  slices);
            
            generate_vertices();
            generate_faces();
            
            std::cout << "Revolution Vertices: " << vertices.size() << std::endl;
            std::cout << "Revolution Faces: " << faces.size() << std::endl;
        }
        
       ~RevolutionMesh() = default;
        
        void generate_vertices () override;
        void generate_faces    () override;
    };
}
