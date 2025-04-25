//
//  FrameBuffer.cpp
//  FileToOBJ
//
//  Created by Andrés Ragot on 1/4/25.
//

#include "FrameBuffer.hpp"
#include <cassert>
#include "Node.hpp"

#ifndef NDEBUG
#include <iostream>
#endif

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
    
    #ifdef DEBUG
    template < typename Color >
    void FrameBuffer<Color>::print_buffer(Buffer buffer_to_print) const
    {
        const ColorVector * buffer = (buffer_to_print == CURRENT_BUFFER) ? current_buffer : next_buffer;

        // Encabezado de columnas
        std::cout << " ";
        for (size_t col = 0; col < width; ++col)
        {
            std::cout << col << " ";
        }
        std::cout << std::endl;

        // Filas con índice y valores
        for (size_t row = 0; row < height; ++row)
        {
            std::cout << row << " "; // Índice de fila
            for (size_t col = 0; col < width; ++col)
            {
                // Se asume que el buffer está en orden de filas: posición (col, row) en el índice row * width + col
                std::cout << buffer->at(row * width + col) << " ";
            }
            std::cout << std::endl;
        }
    }
    #endif
    
    template class FrameBuffer<RGB565  >;
    template class FrameBuffer<RGB888  >;
    template class FrameBuffer<RGB8    >;

}

