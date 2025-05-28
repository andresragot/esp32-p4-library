//
//  RevolutionMesh.hpp
//  FileToOBJ
//
//  Created by Andrés Ragot on 17/3/25.
//

#pragma once

#include "Mesh.hpp"
#include "Camera.hpp"
#include <iostream>

namespace Ragot
{
    class RevolutionMesh : public Mesh
    {
    protected:
        const Camera & cam;
        bool faces_can_be_quads;
        static constexpr float PI = 3.14159265358979323846f;
    public:
        RevolutionMesh (mesh_info_t & mesh_info, const Camera & cam) : Mesh (mesh_info), cam (cam)
        {
            faces_can_be_quads = (mesh_info.vertex_amount % 8 == 0 || mesh_info.vertex_amount == 4);
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
