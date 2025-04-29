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

#if ESP_PLATFORM != 1
#include <cassert>
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
        
    #if ESP_PLATFORM != 1
        initGLTexture();
    #endif
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
    
    
    #if ESP_PLATFORM != 1
    template < typename Color >
    void FrameBuffer < Color >::initGLTexture()
    {
        if (gl_tex == 0)
        {
            glGenTextures(1, &gl_tex);
            glBindTexture(GL_TEXTURE_2D, gl_tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);

            // Reserva la textura (sin datos aún)
            glTexImage2D(GL_TEXTURE_2D, 0,
                         getGLFormat(),  // formato interno
                         static_cast< GLsizei > (width) ,
                         static_cast< GLsizei > (height),
                         0,
                         getGLFormat(),  // formato fuente
                         getGLType(),    // tipo de dato
                         nullptr);
        }
    }
    
    template<typename Color>
    void FrameBuffer<Color>::sendGL() const
    {
        glBindTexture(GL_TEXTURE_2D, gl_tex);

        // Alineación a 1 byte (importante si width*bytesPorPixel no es múltiplo de 4)
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // Actualiza sólo el contenido (más eficiente que glTexImage2D)
        glTexSubImage2D(GL_TEXTURE_2D, 0,
                        0, 0,
                        static_cast<GLsizei>(width),
                        static_cast<GLsizei>(height),
                        getGLFormat(),
                        getGLType(),
                        current_buffer->data());
        
        GLenum err = glGetError();
        if (err != GL_NO_ERROR)
        {
            std::cerr << "OpenGL error: " << err << "\n";
        }


    }

    // Especializaciones helpers para los tipos que usas:
    template<typename Color>
    GLenum FrameBuffer<Color>::getGLFormat()
    {
        if constexpr (std::is_same_v<Color, RGBA8888>)
            return GL_RGBA;
        else if constexpr (std::is_same_v<Color, RGB888>)
            return GL_RGB;
        else if constexpr (std::is_same_v<Color, RGB565>)
            return GL_RGB;
        else
            return GL_R8;
    }

    template<typename Color>
    GLenum FrameBuffer<Color>::getGLType()
    {
        if constexpr (std::is_same_v<Color, RGBA8888> ||
                      std::is_same_v<Color, RGB888>)
            return GL_UNSIGNED_BYTE;
        else if constexpr (std::is_same_v<Color, RGB565>)
            return GL_UNSIGNED_SHORT_5_6_5;
        else
            return GL_BYTE;
    }
    #endif
    
    
    template class FrameBuffer<RGB565  >;
    template class FrameBuffer<RGB888  >;
    template class FrameBuffer<RGB8    >;

}

