//
//  FrameBuffer.hpp
//  FileToOBJ
//
//  Created by Andrés Ragot on 1/4/25.
//

#pragma once
#include <vector>
#include <cstdint>
#if ESP_PLATFORM == 1
#include "RamAllocator.hpp"
#endif

namespace Ragot
{
    using RGB565   = uint16_t;
    using RGB888   = uint32_t;
    using RGBA8888 = uint32_t;
    using RGB8     = uint8_t ; ///< Color Index

    enum Buffer : uint8_t
    {
        CURRENT_BUFFER  = ( 1 << 0),
        NEXT_BUFFER     = ( 1 << 1),
        MAX_BUFFER      = ( 1 << 2)
    };

    template <typename Color>
    class FrameBuffer
    {
    public:
        using TYPE = Color;
        #if ESP_PLATFORM == 1
        using ColorVector = std::vector < Color, PSRAMAllocator<RGB565, MALLOC_CAP_8BIT > >;
        #else
        using ColorVector = std::vector < Color >;
        #endif
        
    private:

    
        bool double_buffer;
        size_t width;
        size_t height;
        Color color;
        ColorVector   buffer_1;
        ColorVector   buffer_2;
        ColorVector * current_buffer;
        ColorVector * next_buffer;
        
    
    public:
        FrameBuffer (size_t width, size_t height, bool double_buffer);
        FrameBuffer () = delete;
       ~FrameBuffer () = default;
       
        // Nada más queremos que haya un FrameBuffer por ahora.
        FrameBuffer (const FrameBuffer &) = delete;
        FrameBuffer (const FrameBuffer &&) = delete;
        FrameBuffer & operator = (const FrameBuffer &) = delete;
        FrameBuffer & operator = (const FrameBuffer &&) = delete;
        
        void swap_buffers();
        void clear_buffer( Buffer buffer_to_clear = NEXT_BUFFER );
        void fill (Color color = 0, Buffer buffer_to_fill = NEXT_BUFFER);
        void set_pixel (size_t x, size_t y, Color color);
        void set_pixel (size_t offset, Color color);
        void set_pixel (size_t offset);
        void set_color (Color color);
        Color get_pixel (size_t x, size_t y) const;
        
        size_t get_width ()        { return width;  }
        size_t get_width ()  const { return width;  }
        size_t get_height ()       { return height; }
        size_t get_height () const { return height; }
        
        const Color * get_buffer() const { return current_buffer->data(); }
        
        inline void blit_to_window () const
        {
            // Implementar la función para blit a la ventana
            std::copy(current_buffer->begin(), current_buffer->end(), next_buffer->begin());
        }
        
        #ifdef DEBUG
        void print_buffer ( Buffer buffer_to_print = CURRENT_BUFFER ) const;
        #endif
        
    };
}
