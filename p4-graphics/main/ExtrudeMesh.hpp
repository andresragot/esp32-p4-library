//
//  ExtrudeMesh.hpp
//  FileToOBJ
//
//  Created by Andrés Ragot on 17/3/25.
//

#pragma once

#include "Mesh.hpp"
#include <iostream>

namespace Ragot
{
    class ExtrudeMesh : public Mesh
    {
    private:
        float height = 1.0f;
        bool faces_can_be_quads = false;
    
    public:
        ExtrudeMesh (mesh_info_t & mesh_info) : Mesh (mesh_info)
        {
            vertices.reserve (mesh_info.coordinates.size() * 2    );
               faces.reserve (mesh_info.coordinates.size() * 3 - 3);
            // Si son 14 vertices -> 39 - 28 = 11
            // Si son 12 vertices -> 33 - 24 = 9
            // Si son 10 vertices -> 27 - 20 = 7
            // Si son  8 vertices ->  9 - 16 = -7
            // Si son  6 vertices -> 15 - 12 = 3
            // Si son 4 vertices ->   9 -  5 = 4
            
            // % 8 porque como están las coordenadas duplicadas...
            faces_can_be_quads = (mesh_info.vertex_amount % 8 == 0 || mesh_info.vertex_amount == 4);
            generate_vertices();
            generate_faces();
            
            std::cout << "Etrude Vertices: " << vertices.size() << std::endl;
            std::cout << "Extrude Faces: " << faces.size() << std::endl;
            
        }
        
       ~ExtrudeMesh() = default;
        
        void generate_vertices () override;
        void generate_faces    () override;
        void generate_faces_direct(const Camera& cam);
        
        bool are_vertices_coplanar (const glm::fvec4 & v1, const glm::fvec4 & v2, const glm::fvec4 & v3, const glm::fvec4 & v4, float tolerance = 0.1f);

    };
}
