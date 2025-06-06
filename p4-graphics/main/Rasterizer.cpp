/**
 * @file Rasterizer.cpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief Implementation of the Rasterizer class for rendering polygons in a frame buffer.
 * 
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

#include "Rasterizer.hpp"
#include <stdio.h>
#include <algorithm>

namespace Ragot
{
    template < class COLOR_BUFFER_TYPE > int Rasterizer< COLOR_BUFFER_TYPE >::offset_cache0 [1024];
    template < class COLOR_BUFFER_TYPE > int Rasterizer< COLOR_BUFFER_TYPE >::offset_cache1 [1024];
    
#ifndef CONFIG_GRAPHICS_PAINTER_ALGO_ENABLED
    template < class COLOR_BUFFER_TYPE > int Rasterizer< COLOR_BUFFER_TYPE >::z_cache0 [1024];
    template < class COLOR_BUFFER_TYPE > int Rasterizer< COLOR_BUFFER_TYPE >::z_cache1 [1024];
#endif // CONFIG_GRAPHICS_PAINTER_ALGO_ENABLED
    
    template <typename COLOR >
    void Rasterizer<COLOR>::fill_convex_polygon(const glm::ivec4 * const vertices,
                                                const int        * const indices_begin,
                                                const int        * const indices_end)
    {
        int   pitch = static_cast < int > (frame_buffer.get_width ());
        int * offset_cache0 = this->offset_cache0;
        int * offset_cache1 = this->offset_cache1;

        const int * indices_back = indices_end - 1;
        
      // Se busca el vértice de inicio (el que tiene menor Y) y el de terminación (el que tiene mayor Y):

        const int * start_index   = indices_begin;
              int   start_y       = vertices[*start_index][1];
        const int * end_index     = indices_begin;
              int   end_y         = start_y;
              
        for (const int * index_iterator = start_index; ++index_iterator < indices_end; )
        {
            int current_y = vertices[*index_iterator][1];

            if (current_y < start_y)
            {
                start_y     = current_y;
                start_index = index_iterator;
            }
            else
            if (current_y > end_y)
            {
                end_y       = current_y;
                end_index   = index_iterator;
            }
        }
        
        const int * current_index = start_index;
        const int * next_index    = start_index > indices_begin ? start_index - 1 : indices_back;
        
        int y0 = vertices[*current_index][1];
        int y1 = vertices[*   next_index][1];
        int o0 = vertices[*current_index][0] + y0 * pitch;
        int o1 = vertices[*   next_index][0] + y1 * pitch;
        
        while (true)
        {
            interpolate< int32_t, 11> (offset_cache0, o0, o1, y0, y1);
            
            if (current_index == indices_begin) current_index = indices_back; else current_index--;
            if (current_index == end_index    ) break;
            if (   next_index == indices_begin) next_index = indices_back; else next_index--;
            
            y0 = y1;
            y1 = vertices[*current_index][1];
            o0 = o1;
            o1 = vertices[*next_index][0] + y1 * pitch;
        }
        
        int end_offset = o1;
        
        current_index = start_index;
           next_index = start_index < indices_back ? start_index + 1 : indices_begin;
           
        y0 = vertices[*current_index][1];
        y1 = vertices[*   next_index][1];
        o0 = vertices[*current_index][0] + y0 * pitch;
        o1 = vertices[*   next_index][0] + y1 * pitch;
        
        while (true)
        {
            interpolate< int32_t, 11 > (offset_cache1, o0, o1, y0, y1);
            if (current_index == indices_back) current_index = indices_begin; else current_index++;
            if (current_index == end_index   ) break;
            if (   next_index == indices_back) next_index = indices_begin; else next_index++;
            
            y0 = y1;
            y1 = vertices[*current_index][1];
            o0 = o1;
            o1 = vertices[*next_index][0] + y1 * pitch;
        }
        
        if (o1 > end_offset) end_offset = o1;
        
        offset_cache0 += start_y;
        offset_cache1 += start_y;
        
        auto pixels = frame_buffer.get_buffer();
        for (int y = start_y; y < end_y; ++y)
        {
            o0 = *offset_cache0++;
            o1 = *offset_cache1++;
            
            if (o0 < o1)
            {
                fill_row < sizeof (COLOR) > (pixels, o0, o1, color);
            }
            else
            {
                fill_row < sizeof (COLOR) > (pixels, o1, o0, color);
            }
        }
    }
    
    template < typename COLOR >
    void Rasterizer<COLOR>::fill_convex_polygon(const glm::ivec4 *const vertices, const face_t *const face)
    {
        // Desempaquetamos los índices
        int polygon_indices[4];
        int vertex_count = 3;
        polygon_indices [0] = face->v1;
        polygon_indices [1] = face->v2;
        polygon_indices [2] = face->v3;
        if (face->is_quad)
        {
            polygon_indices [3] = face->v4;
            vertex_count = 4;
        }
        
        const int * indices_begin = polygon_indices;
        const int * indices_end   = polygon_indices + vertex_count;
        
        fill_convex_polygon(vertices, indices_begin, indices_end);
    }

#ifndef CONFIG_GRAPHICS_PAINTER_ALGO_ENABLED
    template <typename COLOR >
    void Rasterizer<COLOR>::fill_convex_polygon_z_buffer(
        const glm::ivec4 * const vertices,
        const face_t * const face
    )
    {
        // Desempaquetamos los índices
        int polygon_indices[4];
        int vertex_count = 3;
        polygon_indices[0] = face->v1;
        polygon_indices[1] = face->v2;
        polygon_indices[2] = face->v3;
        if (face->is_quad)
        {
            polygon_indices[3] = face->v4;
            vertex_count = 4;
        }

        const int * indices_begin = polygon_indices;
        const int * indices_end   = polygon_indices + vertex_count;
        const int * indices_back  = indices_end - 1;

        // Parámetros de pantalla
        int width  = static_cast<int>(frame_buffer.get_width());
        int height = static_cast<int>(frame_buffer.get_height());

        // Determinamos Y mínimo y máximo
        const int * start_index = indices_begin;
        const int * end_index   = indices_begin;
        int start_y = vertices[*start_index][1];
        int end_y   = start_y;
        
        for (const int * it = start_index; ++it < indices_end;)
        {
            int cy = vertices[*it][1];
            if (cy < start_y)
            {
                start_y = cy;
                start_index = it;
            }
            else if (cy > end_y)
            {
                end_y = cy;
                end_index = it;
            }
        }

        // Clamping de Y para caches
        int y_min = std::clamp(start_y, 0, height - 1);
        int y_max = std::clamp(end_y,   0, height - 1);
        if (y_min > y_max) return; // nada que rasterizar

        // Caches iniciales
        int * offset0 = offset_cache0;
        int * offset1 = offset_cache1;
        int * z0c     = z_cache0;
        int * z1c     = z_cache1;

        // Lado izquierdo (anti-horario)
        {
            const int * cur = start_index;
            const int * next = (start_index > indices_begin) ? start_index - 1 : indices_back;
            int y0 = vertices[*cur][1], z0 = vertices[*cur][2];
            int y1 = vertices[*next][1], z1 = vertices[*next][2];
            int o0 = vertices[*cur][0] + y0 * width;
            int o1 = vertices[*next][0] + y1 * width;

            while (true)
            {
                // Interpolamos X y Z con clamp interno
                interpolate<int64_t,32>(offset0, o0, o1, y0, y1);
                interpolate<int32_t,0>(    z0c, z0, z1, y0, y1);

                if (cur == indices_begin) cur = indices_back; else --cur;
                if (cur == end_index) break;
                if (next == indices_begin) next = indices_back; else --next;

                y0 = y1; z0 = z1;
                y1 = vertices[*next][1]; z1 = vertices[*next][2];
                o0 = o1;
                o1 = vertices[*next][0] + y1 * width;
            }
        }

        // Lado derecho (horario)
        {
            const int * cur = start_index;
            const int * next = (start_index < indices_back) ? start_index + 1 : indices_begin;
            int y0 = vertices[*cur][1], z0 = vertices[*cur][2];
            int y1 = vertices[*next][1], z1 = vertices[*next][2];
            int o0 = vertices[*cur][0] + y0 * width;
            int o1 = vertices[*next][0] + y1 * width;

            while (true)
            {
                interpolate<int64_t,32>(offset1, o0, o1, y0, y1);
                interpolate<int32_t,0>(    z1c, z0, z1, y0, y1);

                if (cur == indices_back) cur = indices_begin; else ++cur;
                if (cur == end_index) break;
                if (next == indices_back) next = indices_begin; else ++next;

                y0 = y1; z0 = z1;
                y1 = vertices[*next][1]; z1 = vertices[*next][2];
                o0 = o1;
                o1 = vertices[*next][0] + y1 * width;
            }
        }

        // Relleno con Z-buffer, recorriendo solo [y_min..y_max]
        for (int y = y_min; y <= y_max; ++y)
        {
            int o_start = offset0[y];
            int o_end   = offset1[y];
            int z_start = z0c[y];
            int z_end   = z1c[y];

            if (o_start == o_end) continue;

            // Clamping de offsets a rango válido de frame_buffer
            int fb_size = width * height;
            o_start = std::clamp(o_start, 0, fb_size - 1);
            o_end   = std::clamp(o_end,   0, fb_size - 1);

            int len = std::abs(o_end - o_start);
            int dz  = (z_end - z_start) / (len > 0 ? len : 1);

            auto pixels = this->frame_buffer.get_buffer();
            auto zbuf   = this->z_buffer.data();

           if (o_start < o_end)
           {
                fill_row_zbuffer < sizeof(COLOR) >(
                    pixels, zbuf,
                    o_start, o_end,
                    z_start, dz,
                    color
                );
            }
            else
            {
                // si va decreciendo, invierte el tramo y el signo de dz:
                fill_row_zbuffer < sizeof(COLOR) >(
                    pixels, zbuf,
                    o_end, o_start,
                    z_end,   -dz,
                    color
                );
            }
        }
    }
    
    template < typename COLOR >
    void Rasterizer< COLOR >::fill_convex_polygon_z_buffer
        (
            const glm::ivec4 * const vertices, 
            const int     * const indices_begin, 
            const int     * const indices_end
        )
    {
        // Se cachean algunos valores de interés:

                int   pitch         = static_cast< int >(frame_buffer.get_height ());
                int * offset_cache0 = this->offset_cache0;
                int * offset_cache1 = this->offset_cache1;
                int * z_cache0      = this->z_cache0;
                int * z_cache1      = this->z_cache1;
        const int * indices_back  = indices_end - 1;

        // Se busca el vértice de inicio (el que tiene menor Y) y el de terminación (el que tiene mayor Y):

        const int * start_index   = indices_begin;
                int   start_y       = vertices[*start_index][1];
        const int * end_index     = indices_begin;
                int   end_y         = start_y;

        for (const int * index_iterator = start_index; ++index_iterator < indices_end; )
        {
            int current_y = vertices[*index_iterator][1];

            if (current_y < start_y)
            {
                start_y     = current_y; 
                start_index = index_iterator;
            }
            else
            if (current_y > end_y)
            {
                end_y       = current_y;
                end_index   = index_iterator;
            }
        }

        // Se cachean las coordenadas X de los lados que van desde el vértice con Y menor al
        // vértice con Y mayor en sentido antihorario:

        const int * current_index = start_index;
        const int *    next_index = start_index > indices_begin ? start_index - 1 : indices_back;

        int y0 = vertices[*current_index][1];
        int y1 = vertices[*   next_index][1];
        int z0 = vertices[*current_index][2];
        int z1 = vertices[*   next_index][2];
        int o0 = vertices[*current_index][0] + y0 * pitch;
        int o1 = vertices[*   next_index][0] + y1 * pitch;

        while (true)
        {
            interpolate< int64_t, 32 > (offset_cache0, o0, o1, y0, y1);
            interpolate< int32_t,  0 > (     z_cache0, z0, z1, y0, y1);

            if (current_index == indices_begin) current_index = indices_back; else current_index--;
            if (current_index == end_index    ) break;
            if (   next_index == indices_begin) next_index    = indices_back; else    next_index--;

            y0 = y1;
            y1 = vertices[*next_index][1];
            z0 = z1;
            z1 = vertices[*next_index][2];
            o0 = o1;
            o1 = vertices[*next_index][0] + y1 * pitch;
        }

        int end_offset = o1;

        // Se cachean las coordenadas X de los lados que van desde el vértice con Y menor al
        // vértice con Y mayor en sentido horario:

        current_index = start_index;
            next_index = start_index < indices_back ? start_index + 1 : indices_begin;

        y0 = vertices[*current_index][1];
        y1 = vertices[*   next_index][1];
        z0 = vertices[*current_index][2];
        z1 = vertices[*   next_index][2];
        o0 = vertices[*current_index][0] + y0 * pitch;
        o1 = vertices[*   next_index][0] + y1 * pitch;

        while (true)
        {
            interpolate< int64_t, 32 > (offset_cache1, o0, o1, y0, y1);
            interpolate< int32_t,  0 > (     z_cache1, z0, z1, y0, y1);

            if (current_index == indices_back) current_index = indices_begin; else current_index++;
            if (current_index == end_index   ) break;
            if (   next_index == indices_back) next_index    = indices_begin; else next_index++;

            y0 = y1;
            y1 = vertices[*next_index][1];
            z0 = z1;
            z1 = vertices[*next_index][2];
            o0 = o1;
            o1 = vertices[*next_index][0] + y1 * pitch;
        }

        if (o1 > end_offset) end_offset = o1;

        // Se rellenan las scanlines desde la que tiene menor Y hasta la que tiene mayor Y:

        offset_cache0 += start_y;
        offset_cache1 += start_y;
        z_cache0 += start_y;
        z_cache1 += start_y;

        for (int y = start_y; y < end_y; y++)
        {
            o0 = *offset_cache0++;
            o1 = *offset_cache1++;
            z0 = *z_cache0++;
            z1 = *z_cache1++;

            if (o0 < o1)
            {
                int z_step = (z1 - z0) / (o1 - o0);

                while (o0 < o1)
                {
                    if (z0 < z_buffer[o0])
                    {
                        frame_buffer.set_pixel (o0, color);
                        z_buffer[o0] = z0;
                    }

                    z0 += z_step;
                    o0++;
                }

                if (o0 > end_offset) break;
            }
            else
            if (o1 < o0)
            {
                int z_step = (z0 - z1) / (o0 - o1);

                while (o1 < o0)
                {
                    if (z1 < z_buffer[o1])
                    {
                        frame_buffer.set_pixel (o1, color);
                        z_buffer[o1] = z1;
                    }

                    z1 += z_step;
                    o1++;
                }

                if (o1 > end_offset) break;
            }
        }
    }
#endif // CONFIG_GRAPHICS_PAINTER_ALGO_ENABLED
    
    template < typename COLOR >
    template < typename VALUE_TYPE, size_t SHIFT >
    void Rasterizer<COLOR>::interpolate
    (
        int * cache,
        int   v0,
        int   v1,
        int   y_min,
        int   y_max
    )
    {
        if (y_max <= y_min) return;
        // Ajuste de límites para el array de 1024
        int ym = std::clamp(y_min, 0, 1023);
        int yM = std::clamp(y_max, 0, 1023);

        VALUE_TYPE value = static_cast<VALUE_TYPE>(v0) << SHIFT;
        VALUE_TYPE step  = (static_cast<VALUE_TYPE>(v1 - v0) << SHIFT) / (y_max - y_min);
        for (int y = ym; y <= yM; ++y)
        {
            cache[y] = static_cast<int>(value >> SHIFT);
            value += step;
        }
    }
    

    template < >
    template < >
    void Rasterizer<RGB565>::fill_row < 2 > (RGB565 * start, unsigned left_offset, unsigned right_offset, const RGB565 & color)
    {
        unsigned length = right_offset - left_offset;
        
        if (length < 128)
        {
            std::fill_n (start + left_offset, length, color);
        }
        else
        {
            uint8_t * pixel   = reinterpret_cast< uint8_t * >(start + left_offset);
            uint8_t * left64  = pixel + ( left_offset & 7) + 8;
            uint8_t * right64 = pixel + (right_offset & 7);
            uint8_t * end     = pixel + length * 2;
            
            uint64_t color_pack = static_cast < uint64_t > (*reinterpret_cast< const uint16_t * >(&color));
            
            color_pack |= color_pack << 16;
            color_pack |= color_pack << 32;
            
            for ( ; pixel < left64 ; pixel += 2) *reinterpret_cast< RGB565    * >(pixel) = color;
            for ( ; pixel < right64; pixel += 8) *reinterpret_cast< uint64_t * >(pixel) = color_pack;
            for ( ; pixel < end    ; pixel += 2) *reinterpret_cast< RGB565    * >(pixel) = color;
        }
    }
    
    // Especialización opcional para COLOR_SIZE==2 si quieres forzar inline u otras micro-optimizaciones:
    // aquí sólo lo duplicamos para que el compilador genere un fill_row_zbuffer<2>
    template < >
    template < >
    void Rasterizer<RGB565>::fill_row_zbuffer<2>(
        RGB565 *      start,
        int   *      zbuffer,
        unsigned     left_offset,
        unsigned     right_offset,
        int          z_start,
        int          dz,
        const RGB565 & color
    )
    {
        // idéntico al genérico
        unsigned length = right_offset - left_offset;
        RGB565 * pix = start   + left_offset;
        int   * zb  = zbuffer + left_offset;
        for (unsigned i = 0; i < length; ++i, ++pix, ++zb, z_start += dz)
        {
            if (z_start < *zb)
            {
                *pix = color;
                *zb  = z_start;
            }
        }
    }
    
#ifndef CONFIG_GRAPHICS_PAINTER_ALGO_ENABLED
    template void Rasterizer<RGB565>::fill_convex_polygon_z_buffer(const glm::ivec4* const, const face_t* const);
    template void Rasterizer<RGB565>::fill_convex_polygon_z_buffer(const glm::ivec4 *const vertices, const int *const indices_begin, const int *const indices_end);
#endif // CONFIG_GRAPHICS_PAINTER_ALGO_ENABLED

}
