//
//  Rasterizer.hpp
//  FileToOBJ
//
//  Created by Andrés Ragot on 9/4/25.
//

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
    template <typename Color>
    class Rasterizer
    {
    
    private:
        FrameBuffer < Color > & frame_buffer;

        static int offset_cache0 [1024];
        static int offset_cache1 [1024];
#ifndef CONFIG_GRAPHICS_PAINTER_ALGO_ENABLED
        
        
        std::vector < int > z_buffer;
#endif // CONFIG_GRAPHICS_PAINTER_ALGO_ENABLED
        
        
        Color color;
        Color clear_color;
        
        static constexpr const char * RASTER_TAG = "Rasterizer";
    
    public:
    
        Rasterizer (FrameBuffer < Color > & frame) : frame_buffer (frame)
#ifndef CONFIG_GRAPHICS_PAINTER_ALGO_ENABLED
        , z_buffer(frame.get_width () * frame.get_height ())
#endif // CONFIG_GRAPHICS_PAINTER_ALGO_ENABLED
        {
        }
        
        Rasterizer () = default;
       ~Rasterizer () = default;
        
    public:
        const FrameBuffer < Color > & get_frame_buffer() const
        {
            return (frame_buffer);
        }
        
        void set_color (const Color & new_color)
        {
            color = new_color;
            // frame_buffer.set_color (new_color);
        }
        
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
        
        void fill_convex_polygon (  const glm::ivec4 * const vertices,
                                    const int        * const indices_begin,
                                    const int        * const indices_end
                                 );

        void fill_convex_polygon ( const glm::ivec4 * const vertices,
                                      const face_t     * const face
                                 );

#ifndef CONFIG_GRAPHICS_PAINTER_ALGO_ENABLED
        void fill_convex_polygon_z_buffer ( const glm::ivec4 * const vertices,
                                            const face_t     * const face
                                          );*/

        void fill_convex_polygon_z_buffer (
                                            const glm::ivec4 * const vertices, 
                                            const int     * const indices_begin, 
                                            const int     * const indices_end
                                          );
#endif // CONFIG_GRAPHICS_PAINTER_ALGO_ENABLED
                                          
        // Logs debug para rasterizado
        bool debug_enabled = true;
                                          
    private:
        
        template < typename VALUE_TYPE, size_t SHIFT >
        void interpolate (int * cache, int v0, int v1, int y_min, int y_max);
        
        template < unsigned COLOR_SIZE >
        void fill_row (Color * start, unsigned left_offset, unsigned right_offset, const Color & color)
        {
            std::fill_n (start + left_offset, right_offset - left_offset, color);
        }

        // dentro de Rasterizer<COLOR> o en un header común
        template <unsigned COLOR_SIZE>
        void fill_row_zbuffer(
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
    
    template class Rasterizer<RGB565>;
}
