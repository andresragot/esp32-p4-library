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
#include "esp_log.h"

namespace Ragot
{
    template <typename Color>
    class Rasterizer
    {
    
    private:
        FrameBuffer < Color > & frame_buffer;
        
        static int offset_cache0 [1024];
        static int offset_cache1 [1024];
        
        static int z_cache0 [1024];
        static int z_cache1 [1024];
        
        
        Color color;
        
        std::vector < int > z_buffer;
        static constexpr char* RASTER_TAG = "Rasterizer";
    
    public:
    
        Rasterizer (FrameBuffer < Color > & frame) : frame_buffer (frame), z_buffer(frame.get_width () * frame.get_height ())
        {
        }
        
        Rasterizer () = default;
       ~Rasterizer () = default;
        
        const FrameBuffer < Color > & get_frame_buffer() const
        {
            return (frame_buffer);
        }
        
    public:
        void set_color (const Color & new_color)
        {
            color = new_color;
            frame_buffer.set_color (new_color);
        }
        
        void clear ()
        {
            ESP_LOGI(RASTER_TAG, "Limpiando framebuffer");
            frame_buffer.clear_buffer();
            
            for (int * z = z_buffer.data(), * end = z + z_buffer.size(); z != end; ++z)
            {
                * z = std::numeric_limits< int >::max ();
            }
        }
        
        void fill_convex_polygon (  const glm::ivec4 * const vertices,
                                    const int        * const indices_begin,
                                    const int        * const indices_end
                                 );
                                 
        void fill_convex_polygon_z_buffer ( const glm::ivec4 * const vertices,
                                            const face_t     * const face
                                          );

        void fill_convex_polygon_z_buffer (
                                            const glm::ivec4 * const vertices, 
                                            const int     * const indices_begin, 
                                            const int     * const indices_end
                                          );
                                          
        // Logs debug para rasterizado
        bool debug_enabled = true;
                                          
    private:
        
        template < typename VALUE_TYPE, size_t SHIFT >
        void interpolate (int * cache, int v0, int v1, int y_min, int y_max);
    };
    
    template class Rasterizer<RGB565>;
}
