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

        static int offset_cache0 [1024]; ///< Cache for left offsets of scanlines >
        static int offset_cache1 [1024]; ///< Cache for right offsets of scanlines
#ifndef CONFIG_GRAPHICS_PAINTER_ALGO_ENABLED
        
        static int z_cache0 [1024]; ///< Cache for Z-buffer values for left offsets
        static int z_cache1 [1024]; ///< Cache for Z-buffer values for right offsets
        
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
            frame_buffer.clear_buffer();
            
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
                                 );

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
                                 );

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
                                          );
                                        
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
                                          );
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
        void interpolate (int * cache, int v0, int v1, int y_min, int y_max);
        
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
    
    template class Rasterizer<RGB565>; ///< Explicit instantiation of Rasterizer for RGB565 color type
}
