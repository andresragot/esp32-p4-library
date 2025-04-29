/*
 *  This file is part of OpenGL-FinalProject
 *
 *  Developed by Andrés Ragot - github.com/andresragot
 *
 *  MIT License
 *
 *  Copyright (c) 2024 Andrés Ragot
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
 
#include "Window.hpp"
#include <cassert>
#include <glad/glad.h>
#include <SDL_opengl.h>

namespace Ragot
{
    Window::Window
    (
        const char * title,
        int left_x,
        int top_y,
        unsigned width,
        unsigned height,
        const OpenGL_Context_Settings & context_details
    )
    {
        if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
        {
            throw "Failed to initialize the video subsystem.";
        }
        
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, context_details.version_major);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, context_details.version_minor);
        
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,       1);
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
        
        SDL_GL_SetAttribute
        (
            SDL_GL_CONTEXT_PROFILE_MASK,
            context_details.core_profile ? SDL_GL_CONTEXT_PROFILE_CORE :
            SDL_GL_CONTEXT_PROFILE_COMPATIBILITY
        );
        
        if (context_details.depth_buffer_size  ) SDL_GL_SetAttribute (SDL_GL_DEPTH_SIZE,     context_details.depth_buffer_size);
        if (context_details.stencil_buffer_size) SDL_GL_SetAttribute (SDL_GL_STENCIL_SIZE, context_details.stencil_buffer_size);
        
        window_handle = SDL_CreateWindow
        (
            title,
            left_x,
            top_y,
            int(width ),
            int(height),
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
        );
        
        assert (window_handle != nullptr);
        
        opengl_context = SDL_GL_CreateContext (window_handle);

        assert(opengl_context != nullptr);
        
        GLenum glad_is_initialized = gladLoadGL ();
        
        assert(glad_is_initialized);
        
        SDL_GL_SetSwapInterval(context_details.enable_vsync ? 1 : 0);
        
         this->width =  width;
        this->height = height;
    }
    
    Window::~Window()
    {
        if (opengl_context)
        {
            SDL_GL_DeleteContext (opengl_context);
        }

        if (window_handle)
        {
            SDL_DestroyWindow (window_handle);
        }

        SDL_QuitSubSystem (SDL_INIT_VIDEO);
    }
}
