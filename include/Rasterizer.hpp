/**
 * @file Rasterizer.hpp
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

#pragma once
#include "FrameBuffer.hpp"
#include "CommonTypes.hpp"
#include <glm.hpp>
#include <cstdint>
#include <vector>
#include <algorithm>
#include "Logger.hpp"

namespace Ragot
{
    /**
     * @class Rasterizer
     * @brief Class for rasterizing polygons in a frame buffer.
     * 
     * This class provides methods to fill convex polygons in a frame buffer with a specified color.
     * It supports both standard rasterization and Z-buffering techniques.
     * 
     * @tparam Color The color type used for the frame buffer (e.g., RGB565).
     */
    template <typename Color>
    class Rasterizer
    {
    
    private:
        FrameBuffer < Color > & frame_buffer; ///< Reference to the frame buffer where polygons will be drawn

        inline static int offset_cache0[1024] = {};
        inline static int offset_cache1[1024] = {};
#ifndef CONFIG_GRAPHICS_PAINTER_ALGO_ENABLED
        inline static int z_cache0[1024] = {};
        inline static int z_cache1[1024] = {};
        std::vector < int > z_buffer; ///< Z-buffer for depth testing, used when painter's algorithm is disabled
#endif // CONFIG_GRAPHICS_PAINTER_ALGO_ENABLED      
        
        
        Color color; ///< Current color to be used for drawing polygons
        Color clear_color; ///< Color used to clear the frame buffer, default is black
        
        static constexpr const char * RASTER_TAG = "Rasterizer";
    
    public:
        
        /**
         * @brief Constructs a Rasterizer with a given frame buffer.
         * 
         * Initializes the rasterizer with the specified frame buffer and prepares the Z-buffer if needed.
         * 
         * @param frame Reference to the frame buffer where polygons will be drawn.
         */
        Rasterizer (FrameBuffer < Color > & frame) : frame_buffer (frame)
#ifndef CONFIG_GRAPHICS_PAINTER_ALGO_ENABLED
        , z_buffer(frame.get_width () * frame.get_height ())
#endif // CONFIG_GRAPHICS_PAINTER_ALGO_ENABLED
        {
        }
        
        /**
         * @brief Default constructor for the Rasterizer class (Default).
         */
        Rasterizer () = default;

        /**
         * @brief Default destructor for the Rasterizer class.
         * 
         * Cleans up resources used by the rasterizer.
         */
       ~Rasterizer () = default;
        
    public:
        /**
         * @brief Gets the frame buffer associated with this rasterizer.
         * 
         * This method returns a reference to the frame buffer where polygons will be drawn.
         * 
         * @return const FrameBuffer<Color>& Reference to the frame buffer.
         */
        const FrameBuffer < Color > & get_frame_buffer() const
        {
            return (frame_buffer);
        }
        
        /**
         * @brief Gets the current color used for drawing polygons.
         * 
         * This method returns the current color that will be used to fill polygons.
         * 
         * @return const Color& Reference to the current color.
         */
        void set_color (const Color & new_color)
        {
            color = new_color;
            // frame_buffer.set_color (new_color);
        }
        
        /**
         * @brief Gets the current color used for clearing the frame buffer.
         * 
         * This method returns the color that will be used to clear the frame buffer.
         * 
         * @return const Color& Reference to the clear color.
         */
        void clear ()
        {
            logger.Log (RASTER_TAG, 3, "Limpiando framebuffer");
            frame_buffer.clear_buffer(CURRENT_BUFFER);
            
#ifndef CONFIG_GRAPHICS_PAINTER_ALGO_ENABLED
            for (int * z = z_buffer.data(), * end = z + z_buffer.size(); z != end; ++z)
            {
                * z = std::numeric_limits< int >::max ();
            }
#endif // CONFIG_GRAPHICS_PAINTER_ALGO_ENABLED
        }
        
        /**
         * @brief Fills a convex polygon defined by its vertices and indices.
         * 
         * This method fills a convex polygon in the frame buffer using the specified vertices and indices.
         * 
         * @param vertices Pointer to an array of vertices defining the polygon.
         * @param indices_begin Pointer to the beginning of the indices array.
         * @param indices_end Pointer to the end of the indices array.
         */
        void fill_convex_polygon (  const glm::ivec4 * const vertices,
                                    const int        * const indices_begin,
                                    const int        * const indices_end
                                 )
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
            y1 = vertices[*next_index][1];
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
            y1 = vertices[*next_index][1];
            o0 = o1;
            o1 = vertices[*next_index][0] + y1 * pitch;
        }
        
        if (o1 > end_offset) end_offset = o1;
        
        // Clamp Y range to screen bounds and cache array bounds
        int height = static_cast < int > (frame_buffer.get_height ());
        int y_min = std::max(start_y, 0);
        int y_max = std::min(end_y, height);
        if (y_min >= y_max) return;
        
        offset_cache0 += y_min;
        offset_cache1 += y_min;
        
        auto pixels = frame_buffer.get_buffer();
        for (int y = y_min; y < y_max; ++y)
        {
            o0 = *offset_cache0++;
            o1 = *offset_cache1++;
            
            if (o0 > o1) std::swap(o0, o1);
            
            // Clamp per-scanline: offsets must stay within [y*pitch, (y+1)*pitch)
            int row_start = y * pitch;
            int row_end   = row_start + pitch;
            
            if (o0 < row_start) o0 = row_start;
            if (o1 > row_end)   o1 = row_end;
            if (o0 >= o1) continue;
            
            fill_row < sizeof (Color) > (pixels, o0, o1, color);
        }
        }

        /**
         * @brief Fills a convex polygon defined by its vertices and face structure.
         * 
         * This method fills a convex polygon in the frame buffer using the specified vertices and face structure.
         * 
         * @param vertices Pointer to an array of vertices defining the polygon.
         * @param face Pointer to the face structure containing vertex indices.
         */
        void fill_convex_polygon ( const glm::ivec4 * const vertices,
                                      const face_t     * const face
                                 )
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

        /**
         * @brief Fills a convex polygon in the Z-buffer.
         * 
         * This method fills a convex polygon in the Z-buffer using the specified vertices and face structure.
         * It performs depth testing to ensure correct rendering order.
         * 
         * @param vertices Pointer to an array of vertices defining the polygon.
         * @param face Pointer to the face structure containing vertex indices.
         */
        void fill_convex_polygon_z_buffer ( const glm::ivec4 * const vertices,
                                            const face_t     * const face
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
                    fill_row_zbuffer < sizeof(Color) >(
                        pixels, zbuf,
                        o_start, o_end,
                        z_start, dz,
                        color
                    );
                }
                else
                {
                    // si va decreciendo, invierte el tramo y el signo de dz:
                    fill_row_zbuffer < sizeof(Color) >(
                        pixels, zbuf,
                        o_end, o_start,
                        z_end,   -dz,
                        color
                    );
                }
            }
        }
                                        
        /**
         * @brief Fills a convex polygon in the Z-buffer using vertex indices.
         * * This method fills a convex polygon in the Z-buffer using the specified vertices and indices.
         * It performs depth testing to ensure correct rendering order.
         * @param vertices Pointer to an array of vertices defining the polygon.
         * @param indices_begin Pointer to the beginning of the indices array.
         * @param indices_end Pointer to the end of the indices array.
         */
        void fill_convex_polygon_z_buffer (
                                            const glm::ivec4 * const vertices, 
                                            const int     * const indices_begin, 
                                            const int     * const indices_end
                                          )
        {
            // Se cachean algunos valores de interés:

                    int   pitch         = static_cast< int >(frame_buffer.get_width ());
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
                                          
        // Logs debug para rasterizado
        bool debug_enabled = true; /// < Enable or disable debug logging for rasterization operations.
                                          
    private:

        /**
         * @brief Interpolates pixel offsets for a scanline.
         * 
         * This method interpolates pixel offsets for a scanline based on the provided vertex values and Y range.
         * It fills the offset cache with calculated pixel offsets.
         * 
         * @tparam VALUE_TYPE The type of value used for interpolation (e.g., int32_t).
         * @tparam SHIFT The bit shift value used for scaling the interpolation.
         * @param cache Pointer to the cache where interpolated offsets will be stored.
         * @param v0 The starting vertex value.
         * @param v1 The ending vertex value.
         * @param y_min The minimum Y coordinate of the scanline.
         * @param y_max The maximum Y coordinate of the scanline.
         */
        template < typename VALUE_TYPE, size_t SHIFT >
        void interpolate (int * cache, int v0, int v1, int y_min, int y_max)
        {
            if (y_max <= y_min) return;
            // Ajuste de límites para el array de 1024
            int ym = std::clamp(y_min, 0, 1023);
            int yM = std::clamp(y_max, 0, 1023);
            if (ym > yM) return;

            VALUE_TYPE step  = (static_cast<VALUE_TYPE>(v1 - v0) << SHIFT) / (y_max - y_min);
            VALUE_TYPE value = (static_cast<VALUE_TYPE>(v0) << SHIFT) + step * (ym - y_min);
            for (int y = ym; y <= yM; ++y)
            {
                cache[y] = static_cast<int>(value >> SHIFT);
                value += step;
            }
        }
        
        /**
         * @brief Fills a row of pixels in the frame buffer with a specified color.
         * 
         * This method fills a row of pixels in the frame buffer with the specified color, from left_offset to right_offset.
         * 
         * @tparam COLOR_SIZE The size of the color type in bytes.
         * @param start Pointer to the first pixel of the scanline.
         * @param left_offset The starting offset (inclusive).
         * @param right_offset The ending offset (exclusive).
         * @param color The color to fill the row with.
         */
        template < unsigned COLOR_SIZE >
        void fill_row (Color * start, unsigned left_offset, unsigned right_offset, const Color & color)
        {
            std::fill_n (start + left_offset, right_offset - left_offset, color);
        }

        // dentro de Rasterizer<COLOR> o en un header común
        /**
         * @brief Fills a row of pixels in the frame buffer with a specified color using Z-buffering.
         * 
         * This method fills a row of pixels in the frame buffer with the specified color, from left_offset to right_offset,
         * while performing depth testing using the Z-buffer.
         * 
         * @tparam COLOR_SIZE The size of the color type in bytes.
         * @param start Pointer to the first pixel of the scanline.
         * @param zbuffer Pointer to the first element of the Z-buffer.
         * @param left_offset The starting offset (inclusive).
         * @param right_offset The ending offset (exclusive).
         * @param z_start The initial depth value at left_offset.
         * @param dz The increment of depth per pixel.
         * @param color The color to fill the row with.
         */
        template <unsigned COLOR_SIZE>
        void fill_row_zbuffer (
            Color *       start,        // puntero al primer píxel de la scanline
            int   *       zbuffer,      // puntero al primer elemento del Z-buffer
            unsigned      left_offset,  // offset inicial (inclusive)
            unsigned      right_offset, // offset final   (exclusive)
            int           z_start,      // profundidad en left_offset
            int           dz,           // incremento de z por píxel
            const Color & color         // color a pintar
                                    )
        {
            unsigned length = right_offset - left_offset;
            Color * pix = start   + left_offset;
            int   * zb  = zbuffer + left_offset;

            // Recorremos de 0 a length-1
            for (unsigned i = 0; i < length; ++i, ++pix, ++zb, z_start += dz)
            {
                if (z_start < *zb)
                {
                    *pix = color;
                    *zb  = z_start;
                }
            }
        }
    };
}
