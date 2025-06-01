/**
 * @file Assets.hpp
 * @author Andrés Ragpt (github.com/andresragot)
 * @brief This file defines the Assets class, which manages the asset paths for the application.
 * It provides a singleton instance to access asset paths based on the executable's location.
 * The class initializes the base path for assets depending on whether the build is in debug or release mode.
 * It also provides a method to retrieve the full path of an asset by its name.
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

#include <filesystem>
#include <string>

namespace Ragot
{
    using std::string;
    using std::filesystem::path;

    /**
     * @class Assets
     * @brief Manages the asset paths for the application.
     * 
     * The Assets class provides a singleton instance to manage asset paths, allowing for easy retrieval of asset files.
     * It initializes the base path based on the executable file path, and provides a method to get the full path of an asset by its name.
     */
    class Assets
    {
        path base_path = "./";

    public:

        /**
         * @brief Returns the singleton instance of the Assets class.
         * 
         * This method ensures that only one instance of the Assets class exists throughout the application.
         * 
         * @return Assets& Reference to the singleton instance of Assets.
         */
        static Assets & instance()
        {
            static Assets assets;
            return assets;
        }

    private:

        /**
         * @brief Construct a new Assets object
         */
        Assets() = default;
        /**
         * @brief Destroy the Assets object
         */
        Assets(const Assets&) = delete;
        /**
         * @brief Move constructor is deleted to prevent moving the Assets instance.
         */
        Assets(const Assets&&) = delete;
        /**
         * @brief Assignment operator is deleted to prevent copying the Assets instance.
         */
        Assets& operator = (const Assets&) = delete;
        /**
         * @brief Move assignment operator is deleted to prevent moving the Assets instance.
         */
        Assets& operator = (const Assets&&) = delete;

    public:

        /**
         * @brief Initializes the base path for assets based on the executable file path.
         * 
         * This method sets the base path to the "assets" directory relative to the executable's location.
         * In debug mode, it uses a relative path to the assets directory.
         * In release mode, it sets the base path to a specific directory.
         * 
         * @param executable_file_path The path of the executable file.
         */
        void initialize(const string& executable_file_path)
        {
            #if defined NDEBUG
                base_path = path{ executable_file_path }.parent_path() / "assets";
            #else
                base_path = path{ "../../assets/" };
            #endif
        }

        /**
         * @brief Gets the full path of an asset by its name.
         * 
         * This method constructs the full path to an asset file by appending the asset name to the base path.
         * 
         * @param asset_name The name of the asset file.
         * @return path The full path to the asset file.
         */
        path get_asset_path(const string & asset_name)
        {
            return base_path/asset_name;
        }

    };
    
    extern Assets & assets;
}
