/**
 * @file Assets.cpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief This file implements the Assets class, which manages the asset paths for the application.
 * @details The Assets class provides a singleton instance to manage asset paths, allowing for easy retrieval of asset files.
 * It initializes the base path based on the executable file path, and provides a method to get the full path of an asset by its name.
 * 
 * @version 1,0
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
 * 
 */

#include "Assets.hpp"

namespace Ragot
{
    Assets & assets = Assets::instance();
}
