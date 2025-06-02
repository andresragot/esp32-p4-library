/*
 *  This file is part of OpenGL-FinalProject
 *
 *  Developed by Andrés Ragot - github.com/andresragot
 *
 *  MIT License
 *
 *  Copyright (c) 2025 Andrés Ragot
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */
 
#pragma once

#include <SDL.h>
#include <string>
#include <utility>

#include <iostream>

namespace Ragot
{
    /**
     * @brief Class for managing an SDL window with OpenGL context.
     */
    class Window
    {
    public:
        /**
         * @brief Enum for window position.
         */
        enum Position
        {
            UNDEFINED = SDL_WINDOWPOS_UNDEFINED, ///< Undefined position.
            CENTERED  = SDL_WINDOWPOS_CENTERED,  ///< Centered position.
        };
        
        /**
         * @brief Struct for OpenGL context settings.
         */
        struct OpenGL_Context_Settings
        {
            unsigned version_major       = 3;    ///< Major version of OpenGL.
            unsigned version_minor       = 3;    ///< Minor version of OpenGL.
            bool     core_profile        = true; ///< Core profile flag.
            unsigned depth_buffer_size   = 24;   ///< Depth buffer size.
            unsigned stencil_buffer_size = 0;    ///< Stencil buffer size.
            bool     enable_vsync        = true; ///< V-Sync enable flag.
        };

    private:
        SDL_Window* window_handle; ///< Handle to the SDL window.
        SDL_GLContext opengl_context; ///< OpenGL context.
        
        unsigned width;  ///< Width of the window.
        unsigned height; ///< Height of the window.
        
    public:
        /**
         * @brief Constructor for the Window class.
         * @param title Title of the window.
         * @param left_x X coordinate of the window position.
         * @param top_y Y coordinate of the window position.
         * @param width Width of the window.
         * @param height Height of the window.
         * @param context_details OpenGL context settings.
         */
        Window(const std::string& title, int left_x, int top_y, unsigned width, unsigned height, const OpenGL_Context_Settings& context_details)
            : Window(title.c_str(), left_x, top_y, width, height, context_details)
        {
        }

        /**
         * @brief Constructor for the Window class.
         * @param title Title of the window.
         * @param left_x X coordinate of the window position.
         * @param top_y Y coordinate of the window position.
         * @param width Width of the window.
         * @param height Height of the window.
         * @param context_details OpenGL context settings.
         */
        Window(const char* title, int left_x, int top_y, unsigned width, unsigned height, const OpenGL_Context_Settings& context_details);
        
        /**
         * @brief Destructor for the Window class.
         */
        ~Window();
       
    public:
        Window(const Window&) = delete; ///< Deleted copy constructor.
        Window& operator=(const Window&) = delete; ///< Deleted copy assignment operator.
        
        /**
         * @brief Move constructor for the Window class.
         * @param other Other window to move from.
         */
        Window(Window&& other) noexcept
        {
            this->window_handle  = std::exchange(other.window_handle, nullptr);
            this->opengl_context = std::exchange(other.opengl_context, nullptr);
        }

        /**
         * @brief Move assignment operator for the Window class.
         * @param other Other window to move from.
         * @return Reference to the moved window.
         */
        Window& operator=(Window&& other) noexcept
        {
            this->window_handle  = std::exchange(other.window_handle, nullptr);
            this->opengl_context = std::exchange(other.opengl_context, nullptr);
            
            return *this;
        }
        
        /**
         * @brief Swaps the OpenGL buffers.
         */
        void swap_buffers()
        {
            SDL_GL_SwapWindow(window_handle);
        }
        
        /**
         * @brief Gets the width of the window.
         * @return Width of the window.
         */
        unsigned get_width() { return width; }

        /**
         * @brief Gets the height of the window.
         * @return Height of the window.
         */
        unsigned get_height() { return height; }
    };
}
