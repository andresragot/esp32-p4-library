/**
 * @file DebugOverlay.hpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief On-screen HUD with FPS, frame time, mesh / triangle stats and free
 *        heap information for the ESP32-P4 target builds.
 *
 * The overlay only compiles when `CONFIG_GRAPHICS_DEBUG_OVERLAY` is enabled.
 * All overhead disappears when the option is off.
 *
 * @copyright Copyright (c) 2026 Andrés Ragot - MIT License
 */
#pragma once

#include "FrameBuffer.hpp"
#include "Rasterizer.hpp"
#include <cstdint>

namespace Ragot
{
    /**
     * @brief Lightweight HUD for engine diagnostics on the target framebuffer.
     *
     * Per-frame usage:
     *   1. `begin_frame()` clears per-frame counters.
     *   2. While rendering, call `note_*` to bump counters.
     *   3. `end_frame(frame_us)` records timing for FPS smoothing.
     *   4. `draw()` paints the HUD on top of the framebuffer.
     */
    class DebugOverlay
    {
    public:
        DebugOverlay(Rasterizer<RGB565> & r, unsigned w, unsigned h)
        : rast(r), screen_w(w), screen_h(h) {}

        /// Reset per-frame counters before rendering.
        void begin_frame()
        {
            tris_drawn = tris_clipped = tris_culled = faces_total = 0;
            meshes_culled = 0;
        }

        void note_face_drawn()   { ++tris_drawn;   }
        void note_face_clipped() { ++tris_clipped; }
        void note_face_culled()  { ++tris_culled;  }
        void note_face_total()   { ++faces_total;  }
        void note_mesh_culled(size_t face_count)
        {
            ++meshes_culled;
            // Account culled faces in the total / culled counters so HUD stats
            // stay consistent when frustum culling fires.
            faces_total += static_cast<int>(face_count);
            tris_culled += static_cast<int>(face_count);
        }

        /// Feeds frame duration (microseconds) into the FPS smoother.
        void end_frame(uint64_t frame_us)
        {
            const float ms = float(frame_us) * 0.001f;
            // Exponential moving average for stable FPS display.
            ema_frame_ms = (ema_frame_ms <= 0.f) ? ms : (0.9f * ema_frame_ms + 0.1f * ms);
        }

        /// Paint HUD onto the current framebuffer.
        void draw();

    private:
        Rasterizer<RGB565> & rast;
        unsigned screen_w, screen_h;

        int tris_drawn   = 0;
        int tris_clipped = 0;
        int tris_culled  = 0;
        int faces_total  = 0;
        int meshes_culled = 0;
        float ema_frame_ms = 0.f;

        // Glyph cell is 6 pixels wide x 8 pixels tall (5x7 + spacing).
        static constexpr int GLYPH_W = 6;
        static constexpr int GLYPH_H = 8;

        void draw_char(int x, int y, char c, RGB565 fg);
        void draw_text(int x, int y, const char * s, RGB565 fg, RGB565 outline);
    };
}
