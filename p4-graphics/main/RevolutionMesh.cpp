//
//  RevolutionMesh.cpp
//  FileToOBJ
//
//  Created by Andrés Ragot on 10/3/25.
//

#include "RevolutionMesh.hpp"
#include <iostream>

namespace Ragot
{
    using glm::fvec4;
    void RevolutionMesh::generate_vertices()
    {
        vertices.clear();
        
        for (int i = 0; i <= slices; ++i)
        {
            float theta = (2.0f * PI * i) / slices;
            float cos_theta = cos (theta);
            float sin_theta = sin (theta);
            
            for (const auto & coord : mesh_info.coordinates)
            {
                fvec4 v;
                v.x = coord.x * cos_theta;
                v.y = coord.y;
                v.z = coord.x * sin_theta;
                
                vertices.push_back(v);
            }
        }
        
        // for (const auto& vert : vertices)
        {
            // std::cout << "Vertex: (" << vert.x << ", " << vert.y << ", " << vert.z << ")" << std::endl;
        }
    }
    
    void RevolutionMesh::generate_faces()
    {
        faces.clear();
        int coords_size = int(mesh_info.coordinates.size());
        
        for (int i = 0; i < slices; ++i)
        {
            int next_slice = (i + 1);
            for (int j = 0; j < coords_size; ++j)
            {
                int next_j = (j + 1) % coords_size;
                
                face_t face1;
                face1.v1 = next_slice * coords_size + j;
                face1.v2 = i * coords_size + j;
                face1.v3 = i * coords_size + next_j;
                face1.v4 = next_slice * coords_size + next_j;
                face1.is_quad = true;
                
                faces.emplace_back(face1);
            }
        }
    }
}
