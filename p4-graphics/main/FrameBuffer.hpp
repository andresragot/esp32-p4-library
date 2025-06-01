/**
 * @file FrameBuffer.cpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief This file implements the FrameBuffer class, which manages a frame buffer for rendering graphics.
 * @details The FrameBuffer class provides methods to create a frame buffer, swap buffers, clear the buffer, fill it with a color,
 * set and get pixels, and manage OpenGL textures. It supports both single and double buffering modes.
 * The class is designed to be used in graphics applications where rendering performance and buffer management are crucial.
 * @version 0.1
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
 * 
 */

#pragma once
#include <vector>
#include <cstdint>
#if ESP_PLATFORM == 1
#include "RamAllocator.hpp"
#else
#include <glad/glad.h>
#endif

namespace Ragot
{
    using RGB565   = uint16_t;
    using RGB888   = uint32_t;
    using RGBA8888 = uint32_t;
    using RGB8     = uint8_t ; ///< Color Index

    /**
     * @enum Buffer
     * @brief Enum to represent the different buffers in a frame buffer.
     * 
     * This enum defines the constants for the current buffer, next buffer, and maximum buffer.
     */
    enum Buffer : uint8_t
    {
        CURRENT_BUFFER  = ( 1 << 0),
        NEXT_BUFFER     = ( 1 << 1),
        MAX_BUFFER      = ( 1 << 2)
    };

    /**
     * @class FrameBuffer
     * @brief Class to manage a frame buffer for rendering graphics.
     * 
     * This class provides methods to create a frame buffer, swap buffers, clear the buffer, fill it with a color,
     * set and get pixels, and manage OpenGL textures. It supports both single and double buffering modes.
     */
    template <typename Color>
    class FrameBuffer
    {
    public:
        using TYPE = Color;
        #if ESP_PLATFORM == 1
        using ColorVector = std::vector < Color, PSRAMAllocator< Color, MALLOC_CAP_8BIT > >;
        #else
        using ColorVector = std::vector < Color >;
        #endif
        
    private:

    
        bool double_buffer; ///< Flag to indicate if double buffering is enabled
        size_t width; ///< Width of the frame buffer in pixels
        size_t height; ///< Height of the frame buffer in pixels
        Color color; ///< Default color for filling the buffer
        ColorVector   buffer_1; ///< First buffer for single or double buffering
        ColorVector   buffer_2; ///< Second buffer for double buffering (if enabled)
        ColorVector * current_buffer; ///< Pointer to the current buffer being used
        ColorVector * next_buffer; ///< Pointer to the next buffer to be used (for double buffering)
        
    
    public:
        /**
         * @brief Constructor for the FrameBuffer class.
         * 
         * Initializes the frame buffer with the specified width, height, and double buffering option.
         * Allocates memory for the buffers and fills them with the default color.
         * 
         * @param width Width of the frame buffer in pixels.
         * @param height Height of the frame buffer in pixels.
         * @param double_buffer Flag to indicate if double buffering is enabled.
         */
        FrameBuffer (size_t width, size_t height, bool double_buffer);

        /**
         * @brief Default constructor for the FrameBuffer class (Deleted).
         */
        FrameBuffer () = delete;

        /**
         * @brief Default destructor for the FrameBuffer class.
         * 
         * Cleans up resources used by the frame buffer.
         */
       ~FrameBuffer () = default;
        
        // Nada más queremos que haya un FrameBuffer por ahora.

        /**
         * @brief Construct a new Frame Buffer object (Deleted).
         */
        FrameBuffer (const FrameBuffer &) = delete;

        /**
         * @brief Construct a new Frame Buffer object (Deleted).
         */
        FrameBuffer (const FrameBuffer &&) = delete;

        /**
         * @brief Assignment operator for the FrameBuffer class (Deleted).
         * 
         * Prevents assignment of FrameBuffer objects.
         * 
         * @return FrameBuffer& Reference to the current object.
         */
        FrameBuffer & operator = (const FrameBuffer &) = delete;

        /**
         * @brief Assignment operator for the FrameBuffer class (Deleted).
         * 
         * Prevents assignment of FrameBuffer objects.
         * 
         * @return FrameBuffer& Reference to the current object.
         */
        FrameBuffer & operator = (const FrameBuffer &&) = delete;
        
        /**
         * @brief Swaps the current buffer with the next buffer.
         * 
         * This method is used in double buffering to switch between the buffers for rendering.
         */
        void swap_buffers();

        /**
         * @brief Clears the specified buffer by filling it with the default color.
         * 
         * @param buffer_to_clear The buffer to clear (CURRENT_BUFFER, NEXT_BUFFER, or MAX_BUFFER).
         */
        void clear_buffer( Buffer buffer_to_clear = NEXT_BUFFER );

        /**
         * @brief Fills the specified buffer with the given color.
         * 
         * @param color The color to fill the buffer with (default is 0).
         * @param buffer_to_fill The buffer to fill (CURRENT_BUFFER, NEXT_BUFFER, or MAX_BUFFER).
         */
        void fill (Color color = 0, Buffer buffer_to_fill = NEXT_BUFFER);

        /**
         * @brief Sets a pixel at the specified coordinates to the given color.
         * 
         * @param x X coordinate of the pixel.
         * @param y Y coordinate of the pixel.
         * @param color Color value for the pixel.
         */
        void set_pixel (size_t x, size_t y, Color color);

        /**
         * @brief Sets a pixel at the specified offset in the buffer to the given color.
         * 
         * @param offset Offset in the buffer (calculated as y * width + x).
         * @param color Color value for the pixel.
         */
        void set_pixel (size_t offset, Color color);

        /**
         * @brief Sets a pixel at the specified offset in the buffer to the default color.
         * 
         * @param offset Offset in the buffer (calculated as y * width + x).
         */
        void set_pixel (size_t offset);

        /**
         * @brief Sets the default color for the frame buffer.
         * 
         * @param color The color to set as the default.
         */
        void set_color (Color color);

        /**
         * @brief Gets the color of a pixel at the specified coordinates.
         * 
         * @param x X coordinate of the pixel.
         * @param y Y coordinate of the pixel.
         * @return Color The color of the pixel at the specified coordinates.
         */
        Color get_pixel (size_t x, size_t y) const;
        
        /**
         * @brief Gets the color of a pixel at the specified offset in the buffer.
         * 
         * @param offset Offset in the buffer (calculated as y * width + x).
         * @return Color The color of the pixel at the specified offset.
         */
        size_t get_width ()        { return width;  }

        /**
         * @brief Gets the width of the frame buffer.
         * 
         * @return size_t The width of the frame buffer in pixels.
         */
        size_t get_width ()  const { return width;  }

        /**
         * @brief Gets the height of the frame buffer.
         * 
         * @return size_t The height of the frame buffer in pixels.
         */
        size_t get_height ()       { return height; }

        /**
         * @brief Gets the height of the frame buffer.
         * 
         * @return size_t The height of the frame buffer in pixels.
         */
        size_t get_height () const { return height; }
        
        /**
         * @brief Gets the current buffer being used.
         * 
         * @return const Color* Pointer to the current buffer data.
         */
        const Color * get_buffer() const { return current_buffer->data(); }

        /**
         * @brief Get the buffer object
         * 
         * @return Color* 
         */
              Color * get_buffer()       { return current_buffer->data(); }
        
        /**
         * @brief Blits the current buffer to the window.
         * This method copies the contents of the current buffer to the next buffer,
         * effectively preparing the next frame for rendering.
         */
        inline void blit_to_window () const
        {
            // Implementar la función para blit a la ventana
            std::copy(current_buffer->begin(), current_buffer->end(), next_buffer->begin());
        }
        
        
        #if ESP_PLATFORM != 1

        /**
         * @brief Initializes the OpenGL texture for the frame buffer.
         * 
         * This method creates an OpenGL texture and binds it to the frame buffer.
         */
        void initGLTexture();
        
        /**
         * @brief Clears the OpenGL texture associated with the frame buffer.
         * 
         * This method deletes the OpenGL texture to free up resources.
         */
        void sendGL() const;
        
        /**
         * @brief Sets the OpenGL texture for the frame buffer.
         * 
         * This method binds the OpenGL texture to the current context.
         */
        static GLenum getGLFormat();

        /**
         * @brief Gets the OpenGL type for the frame buffer.
         * 
         * This method returns the OpenGL type corresponding to the color format used in the frame buffer.
         * 
         * @return GLenum The OpenGL type for the frame buffer.
         */
        static GLenum getGLType();
        
        /**
         * @brief Gets the OpenGL texture ID for the frame buffer.
         * 
         * @return GLuint The OpenGL texture ID.
         */
        GLuint getGLTex () const { return gl_tex; }
    
    private:
        /**
         * @brief OpenGL texture ID for the frame buffer.
         * 
         * This variable holds the OpenGL texture ID used for rendering the frame buffer.
         */
        GLuint gl_tex = 0;
        #endif
    };
}
