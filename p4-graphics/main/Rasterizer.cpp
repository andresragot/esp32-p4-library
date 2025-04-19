//
//  Rasterizer.cpp
//  FileToOBJ
//
//  Created by Andrés Ragot on 9/4/25.
//

#include "Rasterizer.hpp"
#include <stdio.h>

namespace Ragot
{
    template < class COLOR_BUFFER_TYPE > int Rasterizer< COLOR_BUFFER_TYPE >::offset_cache0 [1024];
    template < class COLOR_BUFFER_TYPE > int Rasterizer< COLOR_BUFFER_TYPE >::offset_cache1 [1024];
    
    template < class COLOR_BUFFER_TYPE > int Rasterizer< COLOR_BUFFER_TYPE >::z_cache0 [1024];
    template < class COLOR_BUFFER_TYPE > int Rasterizer< COLOR_BUFFER_TYPE >::z_cache1 [1024];
    
    template <typename COLOR >
    void Rasterizer<COLOR>::fill_convex_polygon(const glm::ivec4 * const vertices,
                                                const int        * const indices_begin,
                                                const int        * const indices_end)
    {
        int   pitch = frame_buffer.get_height ();
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
            interpolate< int64_t, 32> (offset_cache0, o0, o1, y0, y1);
            
            if (current_index == indices_begin) current_index = indices_back; else current_index--;
            if (current_index == end_index) break;
            if (current_index == indices_begin) next_index = indices_back; else next_index--;
            
            y0 = y1;
            y1 = vertices[*current_index][1];
            o0 = o1;
            o1 = vertices[*next_index][0] + y1 * pitch;
        }
        
        int end_offset = o1;
        
        current_index = start_index;
           next_index = start_index > indices_begin ? start_index - 1 : indices_back;
           
        y0 = vertices[*current_index][1];
        y1 = vertices[*   next_index][1];
        o0 = vertices[*current_index][0] + y0 * pitch;
        o1 = vertices[*   next_index][0] + y1 * pitch;
        
        while (true)
        {
            interpolate< int64_t, 32> (offset_cache1, o0, o1, y0, y1);
            if (current_index == indices_back) current_index = indices_begin; else current_index++;
            if (current_index == end_index) break;
            if (next_index == indices_back) next_index = indices_begin; else next_index++;
            
            y0 = y1;
            y1 = vertices[*current_index][1];
            o0 = o1;
            o1 = vertices[*next_index][0] + y1 * pitch;
        }
        
        if (o1 > end_offset) end_offset = o1;
        
        offset_cache0 += start_y;
        offset_cache1 += start_y;
        
        for (int y = start_y; y < end_y; ++y)
        {
            o0 = *offset_cache0++;
            o1 = *offset_cache1++;
            
            if (o0 < o1)
            {
                while (o0 < o1) frame_buffer.set_pixel(o0++);
                
                if (o0 > end_offset) break;
            }
            else
            {
                while (o1 > o0) frame_buffer.set_pixel(o1++);
                
                if (o1 > end_offset) break;
            }
        }
    }
    
    template <typename COLOR>
    void Rasterizer<COLOR>::fill_convex_polygon_z_buffer(const glm::ivec4 * const vertices, const face_t * const face)
    {
        // Desempaquetamos los índices contenidos en face en un arreglo temporal.
        int polygon_indices[4];
        int vertex_count = 3;  // por defecto es triángulo
        polygon_indices[0] = face->v1;
        polygon_indices[1] = face->v2;
        polygon_indices[2] = face->v3;
        if (face->is_quad)
        {
            polygon_indices[3] = face->v4;
            vertex_count = 4;
        }

        // Creamos el rango de índices a utilizar
        const int * indices_begin = polygon_indices;
        const int * indices_end   = polygon_indices + vertex_count;

        // Se cachean algunos valores de interés:
        int pitch           = static_cast < int > (frame_buffer.get_height());
        int * offset_cache0 = this->offset_cache0;
        int * offset_cache1 = this->offset_cache1;
        int * z_cache0      = this->z_cache0;
        int * z_cache1      = this->z_cache1;
        const int * indices_back = indices_end - 1;

        // Se determina el vértice de inicio (el que tiene menor Y) y de terminación (el que tiene mayor Y)
        const int * start_index = indices_begin;
              int   start_y     = vertices[*start_index][1];
        const int * end_index   = indices_begin;
              int   end_y       = start_y;

        for (const int * index_iterator = start_index; ++index_iterator < indices_end; )
        {
            int current_y = vertices[*index_iterator][1];
            if (current_y < start_y)
            {
                start_y     = current_y;
                start_index = index_iterator;
            }
            else if (current_y > end_y)
            {
                end_y     = current_y;
                end_index = index_iterator;
            }
        }

        // Interpolación de los bordes en sentido antihorario (lado izquierdo)
        const int * current_index = start_index;
        const int * next_index    = (start_index > indices_begin) ? start_index - 1 : indices_back;

        int y0 = vertices[*current_index][1];
        int y1 = vertices[*next_index][1];
        int z0 = vertices[*current_index][2];
        int z1 = vertices[*next_index][2];
        int o0 = vertices[*current_index][0] + y0 * pitch;
        int o1 = vertices[*next_index][0] + y1 * pitch;

        while (true)
        {
            interpolate< int64_t, 32 >(offset_cache0, o0, o1, y0, y1);
            interpolate< int32_t,  0 >(     z_cache0, z0, z1, y0, y1);

            if (current_index == indices_begin)
                current_index = indices_back;
            else
                current_index--;
            if (current_index == end_index)
                break;
            if (next_index == indices_begin)
                next_index = indices_back;
            else
                next_index--;

            y0 = y1;
            y1 = vertices[*next_index][1];
            z0 = z1;
            z1 = vertices[*next_index][2];
            o0 = o1;
            o1 = vertices[*next_index][0] + y1 * pitch;
        }

        int end_offset = o1;

        // Interpolación de los bordes en sentido horario (lado derecho)
        current_index = start_index;
        next_index    = (start_index < indices_back) ? start_index + 1 : indices_begin;

        y0 = vertices[*current_index][1];
        y1 = vertices[*next_index][1];
        z0 = vertices[*current_index][2];
        z1 = vertices[*next_index][2];
        o0 = vertices[*current_index][0] + y0 * pitch;
        o1 = vertices[*next_index][0] + y1 * pitch;

        while (true)
        {
            interpolate< int64_t, 32 >(offset_cache1, o0, o1, y0, y1);
            interpolate< int32_t,  0 >(     z_cache1, z0, z1, y0, y1);

            if (current_index == indices_back)
                current_index = indices_begin;
            else
                current_index++;
            if (current_index == end_index)
                break;
            if (next_index == indices_back)
                next_index = indices_begin;
            else
                next_index++;

            y0 = y1;
            y1 = vertices[*next_index][1];
            z0 = z1;
            z1 = vertices[*next_index][2];
            o0 = o1;
            o1 = vertices[*next_index][0] + y1 * pitch;
        }

        if (o1 > end_offset)
            end_offset = o1;

        // Relleno de las scanlines desde la Y mínima a la Y máxima usando el Z-buffer
        offset_cache0 += start_y;
        offset_cache1 += start_y;
        z_cache0      += start_y;
        z_cache1      += start_y;

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
                        frame_buffer.set_pixel(o0, color);
                        z_buffer[o0] = z0;
                    }
                    z0 += z_step;
                    o0++;
                }
                if (o0 > end_offset)
                    break;
            }
            else if (o1 < o0)
            {
                int z_step = (z0 - z1) / (o0 - o1);

                while (o1 < o0)
                {
                    if (z1 < z_buffer[o1])
                    {
                        frame_buffer.set_pixel(o1, color);
                        z_buffer[o1] = z1;
                    }
                    z1 += z_step;
                    o1++;
                }
                if (o1 > end_offset)
                    break;
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

                int   pitch         = frame_buffer.get_height ();
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


    template void Rasterizer<RGB565>::fill_convex_polygon_z_buffer(const glm::ivec4* const, const face_t* const);
    
    template < typename COLOR >
    template < typename VALUE_TYPE, size_t SHIFT >
    void Rasterizer < COLOR >::interpolate (int * cache, int v0, int v1, int y_min, int y_max)
    {
        if (y_max > y_min)
        {
            VALUE_TYPE value = (VALUE_TYPE (     v0) << SHIFT);
            VALUE_TYPE step  = (VALUE_TYPE (v1 - v0) << SHIFT) / (y_max - y_min);
            
            for (int * iterator = cache + y_min, * end = cache + y_max; iterator <= end; )
            {
               *iterator++ = int(value >> SHIFT);
                value += step;
               *iterator++ = int(value >> SHIFT);
                value += step;
            }
        }
    }
}
