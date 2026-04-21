/**
 * @file FrameBuffer.cpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief This file implements the FrameBuffer class, which manages a frame buffer for rendering graphics.
 * @details The FrameBuffer class provides methods to create a frame buffer, swap buffers, clear the buffer, fill it with a color,
 * set and get pixels, and manage OpenGL textures. It supports both single and double buffering modes.
 * The class is designed to be used in graphics applications where rendering performance and buffer management are crucial.
 * @version 0.1
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

#include "FrameBuffer.hpp"
#include <cassert>
#include "Node.hpp"

namespace Ragot
{
    template < typename Color >
    FrameBuffer < Color >::FrameBuffer (size_t width, size_t height, bool double_buffer)
    :   double_buffer(double_buffer),
        width(width),
        height(height)
    {
        buffer_1.resize (width * height);
        current_buffer = &buffer_1;
        assert(current_buffer);
        
        if (double_buffer)
        {
            buffer_2.resize (width * height);
            next_buffer = &buffer_2;
            
            assert(next_buffer);
        }
        
        fill (0, CURRENT_BUFFER);
    }
    
    template < typename Color >
    void FrameBuffer < Color >::swap_buffers()
    {
        std::swap (current_buffer, next_buffer);
    }
    
    template < typename Color >
    void FrameBuffer < Color >::clear_buffer( Buffer buffer_to_clear )
    {
        fill (0, buffer_to_clear);
    }
    
    template < typename Color >
    void FrameBuffer < Color >::fill (Color color, Buffer buffer_to_fill)
    {
        switch (buffer_to_fill)
        {
            case CURRENT_BUFFER:
                std::fill (current_buffer->begin(), current_buffer->end(), color);
                break;
                
            case NEXT_BUFFER:
                std::fill (next_buffer->begin(), next_buffer->end(), color);
                break;
                
            case MAX_BUFFER:
                std::fill (current_buffer->begin(), current_buffer->end(), color);
                std::fill (next_buffer->begin(), next_buffer->end(), color);
                break;
        }
    }
    
    template < typename Color >
    void FrameBuffer < Color >::set_pixel (size_t x, size_t y, Color color)
    {
        assert( y * width + x < (width * height));
    
        (*current_buffer)[y * width + x] = color;
    }
    
    template < typename Color >
    void FrameBuffer < Color >::set_pixel(size_t offset, Color color)
    {
        assert(offset < (width * height));
    
        (*current_buffer)[offset] = color;
    }
    
    template < typename Color >
    void FrameBuffer < Color >::set_pixel(size_t offset)
    {
        assert(offset < (width * height));
    
        (*current_buffer)[offset] = color;
    }
        
    
    template < typename Color >
    Color FrameBuffer < Color >::get_pixel (size_t x, size_t y) const
    {
        return (*current_buffer)[y * width + x];
    }
    
    template < typename Color >
    void FrameBuffer < Color >::set_color(Color color)
    {
        this->color = color;
    }
    
    
    template class FrameBuffer<RGB565  >;
    template class FrameBuffer<RGB888  >;
    template class FrameBuffer<RGB8    >;

}

