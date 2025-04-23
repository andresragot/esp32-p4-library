//
//  Mesh.hpp
//  FileToOBJ
//
//  Created by Andrés Ragot on 10/3/25.
//

#pragma once

#include "CommonTypes.hpp"
#include "Camera.hpp"
#include <vector>
#include "Components.hpp"
#include <glm.hpp>

namespace Ragot
{
    class Mesh : public Component
    {
    protected:
        mesh_info_t mesh_info;
        
        std::vector < glm::fvec4 > vertices;
        std::vector <   face_t > faces;
        
        int slices = 4;
        
    public:
        Mesh() = delete;
        virtual ~Mesh() = default;
       
        Mesh(mesh_info_t & mesh_info);
        
        virtual void generate_vertices() = 0;
        virtual void generate_faces() = 0;
        
        const std::vector < glm::fvec4 > & get_vertices() const { return vertices; }
        const std::vector <   face_t > & get_faces()    const { return faces;    }
        
        const size_t get_total_vertices() const { return vertices.size(); }
              size_t get_total_vertices()       { return vertices.size(); }

        void recalculate()
        {
            vertices.clear();
            faces.clear();

            generate_vertices();
            generate_faces();
        }
    };
}
