/**
 * @file Light.hpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief Directional light for per-face diffuse shading.
 * @version 1.0
 * @date 2025-06-01
 * 
 * @copyright Copyright (c) 2025
 * MIT License
 */

#pragma once

#include <glm.hpp>
#include "FrameBuffer.hpp"
#include <algorithm>

namespace Ragot
{
    /**
     * @struct DirectionalLight
     * @brief A simple directional light used for diffuse (Lambert) shading.
     */
    struct DirectionalLight
    {
        glm::vec3 direction;   ///< Normalized direction *toward* the light in view/screen space. Fixed relative to the viewer.
        float     ambient;     ///< Ambient coefficient [0..1]. Minimum brightness so no face goes black.
        float     diffuse;     ///< Diffuse coefficient [0..1]. Strength of the directional component.

        DirectionalLight()
            : direction(glm::normalize(glm::vec3(0.5f, 1.0f, 0.3f)))
            , ambient(0.15f)
            , diffuse(0.85f)
        {}

        DirectionalLight(const glm::vec3 & dir, float amb, float diff)
            : direction(glm::normalize(dir))
            , ambient(amb)
            , diffuse(diff)
        {}
    };

    /**
     * @brief Applies a lighting intensity factor to an RGB565 color.
     *
     * Extracts R5 G6 B5 channels, scales each by @p intensity, and repacks.
     * The intensity is clamped to [0, 1].
     *
     * @param base_color  The original RGB565 color of the mesh.
     * @param intensity   Lighting intensity in [0..1].
     * @return RGB565     The lit color.
     */
    inline RGB565 apply_light_rgb565(RGB565 base_color, float intensity)
    {
        intensity = std::max(0.0f, std::min(1.0f, intensity));

        uint16_t r = (base_color >> 11) & 0x1F;
        uint16_t g = (base_color >>  5) & 0x3F;
        uint16_t b =  base_color        & 0x1F;

        r = static_cast<uint16_t>(r * intensity + 0.5f);
        g = static_cast<uint16_t>(g * intensity + 0.5f);
        b = static_cast<uint16_t>(b * intensity + 0.5f);

        if (r > 0x1F) r = 0x1F;
        if (g > 0x3F) g = 0x3F;
        if (b > 0x1F) b = 0x1F;

        return static_cast<RGB565>((r << 11) | (g << 5) | b);
    }

    /**
     * @brief Computes the diffuse lighting intensity for a face.
     *
     * Uses Lambert's cosine law: intensity = ambient + diffuse * max(dot(N, L), 0).
     * The result is clamped to [0, 1].
     *
     * @param face_normal_world  The face normal in world space (must be normalized).
     * @param light              The directional light parameters.
     * @return float             Intensity in [0..1].
     */
    inline float compute_diffuse_intensity(const glm::vec3 & face_normal_world,
                                           const DirectionalLight & light)
    {
        float NdotL = glm::dot(face_normal_world, light.direction);
        float intensity = light.ambient + light.diffuse * std::max(NdotL, 0.0f);
        return std::min(intensity, 1.0f);
    }
}
