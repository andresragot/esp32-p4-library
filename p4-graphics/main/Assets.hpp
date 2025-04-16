//
//  Assets.hpp
//  FileToOBJ
//
//  Created by Andrés Ragot on 3/2/25.
//

#pragma once

#include <filesystem>
#include <string>

namespace Ragot
{
    using std::string;
    using std::filesystem::path;

    class Assets
    {
        path base_path = "./";

    public:

        static Assets & instance()
        {
            static Assets assets;
            return assets;
        }

    private:

        Assets() = default;
        Assets(const Assets&) = delete;
        Assets(const Assets&&) = delete;
        Assets& operator = (const Assets&) = delete;
        Assets& operator = (const Assets&&) = delete;

    public:

        void initialize(const string& executable_file_path)
        {
            #if defined NDEBUG
                base_path = path{ executable_file_path }.parent_path() / "assets";
            #else
                base_path = path{ "../../assets/" };
            #endif
        }

        path get_asset_path(const string & asset_name)
        {
            return base_path/asset_name;
        }

    };
    
    extern Assets & assets;
}
