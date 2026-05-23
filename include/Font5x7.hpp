/**
 * @file Font5x7.hpp
 * @brief Minimal public-domain 5x7 bitmap font for the debug overlay.
 *
 * Each glyph is 5 columns x 7 rows packed as 5 bytes; bit 0 = top row,
 * bit 6 = bottom row. Cell size when rendered is 6x8 (one column / one
 * row of spacing). Only the characters actually used by the HUD are
 * defined; missing characters fall back to a small filled box.
 *
 * @copyright Public domain / CC0
 */
#pragma once

#include <cstdint>

namespace Ragot
{
    struct Glyph5x7 { uint8_t col[5]; };

    // Layout: each byte is one column (5 cols/char). Bit 0 = top row.
    // Example for 'I':
    //   col0 = 0b1000001  -> row 0 and row 6 lit (top & bottom bars of I)
    //
    // The glyph table is indexed by ASCII code (32..127). Undefined entries
    // render as a 3x5 box.
    inline constexpr Glyph5x7 font5x7_default = { { 0x7F, 0x41, 0x41, 0x41, 0x7F } };

    // Convenience: get the glyph for an ASCII char. Returns the default
    // box for unsupported characters so the HUD never crashes.
    Glyph5x7 font5x7_get(char c);
}
