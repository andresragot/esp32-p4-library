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
}
