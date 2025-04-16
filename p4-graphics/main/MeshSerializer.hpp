//
//  MeshSerializer.hpp
//  FileToOBJ
//
//  Created by Andrés Ragot on 10/3/25.
//

#pragma once

#include "Mesh.hpp"
#include <filesystem>

namespace Ragot
{
    // Vamos a hacer un Singleton.
    class MeshSerializer
    {
    public:
        static MeshSerializer & instance()
        {
            static MeshSerializer serializer;
            return serializer;
        }
        
    private:
    
        MeshSerializer () = default;
        MeshSerializer (const MeshSerializer &)  = delete;
        MeshSerializer (const MeshSerializer &&) = delete;
        MeshSerializer & operator = (const MeshSerializer &)  = delete;
        MeshSerializer & operator = (const MeshSerializer &&) = delete;
        
    public:
        bool save_to_obj (const Mesh & mesh, const std::filesystem::path & path);
        
    };
    
    extern MeshSerializer & serializer;
}
