/**
 * @file DebugOverlay.cpp
 * @brief Implementation of the on-screen debug HUD.
 *
 * @copyright Copyright (c) 2026 Andrés Ragot - MIT License
 */

#include "DebugOverlay.hpp"
#include "Font5x7.hpp"
#include <cstdio>
#include <cstring>

#if defined(ESP_PLATFORM) && ESP_PLATFORM == 1
#include "esp_heap_caps.h"
#endif

namespace Ragot
{
    void DebugOverlay::draw_char(int x, int y, char c, RGB565 fg)
    {
        Glyph5x7 g = font5x7_get(c);
        for (int col = 0; col < 5; ++col)
        {
            uint8_t bits = g.col[col];
            for (int row = 0; row < 7; ++row)
            {
                if (bits & (1u << row))
                    rast.draw_pixel(x + col, y + row, fg);
            }
        }
    }

    void DebugOverlay::draw_text(int x, int y, const char * s, RGB565 fg, RGB565 outline)
    {
        // First pass: outline at 8 neighbour offsets so text reads on any background.
        for (int dy = -1; dy <= 1; ++dy)
            for (int dx = -1; dx <= 1; ++dx)
            {
                if (dx == 0 && dy == 0) continue;
                int cx = x + dx;
                for (const char * p = s; *p; ++p, cx += GLYPH_W)
                    draw_char(cx, y + dy, *p, outline);
            }
        // Second pass: foreground text on top.
        int cx = x;
        for (const char * p = s; *p; ++p, cx += GLYPH_W)
            draw_char(cx, y, *p, fg);
    }

    void DebugOverlay::draw()
    {
#ifdef CONFIG_GRAPHICS_DEBUG_OVERLAY
        const RGB565 white = 0xFFFF;
        const RGB565 black = 0x0000;

        const int x = 2;
        int y = 2;
        char buf[48];

#ifdef CONFIG_GRAPHICS_DEBUG_SHOW_FPS
        {
            float fps = (ema_frame_ms > 0.f) ? (1000.f / ema_frame_ms) : 0.f;
            std::snprintf(buf, sizeof(buf), "FPS %5.1f  %5.1fms", fps, ema_frame_ms);
            draw_text(x, y, buf, white, black);
            y += GLYPH_H + 1;
        }
#endif

#ifdef CONFIG_GRAPHICS_DEBUG_SHOW_STATS
        {
            std::snprintf(buf, sizeof(buf), "faces %d/%d", tris_drawn, faces_total);
            draw_text(x, y, buf, white, black);
            y += GLYPH_H + 1;
            std::snprintf(buf, sizeof(buf), "cull %d clip %d", tris_culled, tris_clipped);
            draw_text(x, y, buf, white, black);
            y += GLYPH_H + 1;
            std::snprintf(buf, sizeof(buf), "mesh cull %d", meshes_culled);
            draw_text(x, y, buf, white, black);
            y += GLYPH_H + 1;
        }
#endif

#ifdef CONFIG_GRAPHICS_DEBUG_SHOW_MEMORY
        {
#if defined(ESP_PLATFORM) && ESP_PLATFORM == 1
            size_t free_int   = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
            size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
            std::snprintf(buf, sizeof(buf), "heap %u K", unsigned(free_int / 1024));
            draw_text(x, y, buf, white, black);
            y += GLYPH_H + 1;
            std::snprintf(buf, sizeof(buf), "psram %u K", unsigned(free_psram / 1024));
            draw_text(x, y, buf, white, black);
            y += GLYPH_H + 1;
#endif
        }
#endif
        (void)x; (void)y; (void)buf;
#endif // CONFIG_GRAPHICS_DEBUG_OVERLAY
    }
}
