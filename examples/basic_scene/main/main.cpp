/**
 * @file main.cpp
 * @brief Basic scene example for the ESP32 3D Engine.
 *
 * This example creates a scene with various 3D objects (revolution and extrusion meshes),
 * sets up a camera that orbits the scene, and renders everything on the display.
 *
 * Supported targets: ESP32-P4 (EK79007 display), ESP32-S3 (ST7262 display).
 */

#include "Renderer.hpp"
#include <vector>
#include "CommonTypes.hpp"
#include "Scene.hpp"
#include "RevolutionMesh.hpp"
#include "ExtrudeMesh.hpp"
#include "Id.hpp"
#include "Logger.hpp"
#include "Thread_Pool.hpp"
#include "Light.hpp"

#ifdef CONFIG_IDF_TARGET_ESP32P4
#include "driver_ek79007.hpp"
#elif CONFIG_IDF_TARGET_ESP32S3
#include "Driver_ST7262.hpp"
#endif

static const char* MAIN_TAG = "Main";
constexpr unsigned SCREEN_W = 320;
constexpr unsigned SCREEN_H = 240;

using namespace std;
using namespace Ragot;

/**
 * @brief Main rendering loop (used when parallel mode is disabled).
 */
void main_loop(Renderer & renderer, Scene & scene)
{
    unsigned frame_count = 0;

    auto last_tick  = std::chrono::high_resolution_clock::now();
    auto current_tick = last_tick;
    std::chrono::high_resolution_clock::duration elapsed_time{};

    while (true)
    {
        current_tick = std::chrono::high_resolution_clock::now();
#ifndef CONFIG_GRAPHICS_PARALLEL_ENABLED
        scene.update(elapsed_time.count());
        renderer.render();
#endif
        frame_count++;
        elapsed_time = current_tick - last_tick;
        last_tick     = current_tick;
    }
}

namespace
{
    /**
     * @brief Populates the scene with several revolution and extrusion meshes.
     */
    void setupScene(Ragot::Scene & scene, Ragot::Camera & camera)
    {
        using namespace Ragot;
        using namespace glm;

        // 1) Cylinder (revolution)
        {
            vector<coordinates_t> coords = {
                { 1.0f, 0.0f },
                { 1.0f, 2.0f },
                { 0.0f, 2.0f }
            };
            mesh_info_t info(coords, RENDER_REVOLUTION);
            auto mesh = std::make_shared<RevolutionMesh>(info, camera);
            mesh->set_position(vec3(0.0f, 0.0f, 0.0f));
            mesh->set_color(0xB345);
            scene.add_node(mesh, ID(CILINDRO));
        }

        // 2) Cone (revolution)
        {
            vector<coordinates_t> coords = {
                { 1.0f, 0.0f },
                { 0.0f, 2.0f },
                { 0.0f, 0.0f }
            };
            mesh_info_t info(coords, RENDER_REVOLUTION);
            auto mesh = std::make_shared<RevolutionMesh>(info, camera);
            mesh->set_position(vec3(3.0f, 0.0f, 0.0f));
            mesh->set_color(0x8410);
            scene.add_node(mesh, ID(CONO));
        }

        // 3) Vase (revolution)
        {
            vector<coordinates_t> coords = {
                { 0.0f, 0.0f },
                { 2.0f, 0.0f },
                { 2.0f, 3.0f },
                { 1.5f, 3.5f },
                { 1.0f, 4.0f },
                { 0.8f, 4.2f },
                { 0.0f, 4.2f }
            };
            mesh_info_t info(coords, RENDER_REVOLUTION);
            auto mesh = std::make_shared<RevolutionMesh>(info, camera);
            mesh->set_position(vec3(-3.0f, 0.0f, 0.0f));
            mesh->set_color(0xF800);
            scene.add_node(mesh, ID(VASO));
        }

        // 4) Hemisphere (revolution)
        {
            vector<coordinates_t> coords = {
                { 0.0f, 0.0f },
                { 0.2f, 0.1f },
                { 0.4f, 0.3f },
                { 0.6f, 0.6f },
                { 0.8f, 1.0f },
                { 1.0f, 1.5f },
                { 0.8f, 1.9f },
                { 0.6f, 2.1f },
                { 0.4f, 2.3f },
                { 0.2f, 2.4f },
                { 0.0f, 2.5f }
            };
            mesh_info_t info(coords, RENDER_REVOLUTION);
            auto mesh = std::make_shared<RevolutionMesh>(info, camera);
            mesh->set_position(vec3(0.0f, 2.5f, 0.0f));
            mesh->set_color(0x07F0);
            scene.add_node(mesh, ID(SEMIESFERA));
        }

        // 5) Toroid (revolution)
        {
            vector<coordinates_t> coords = {
                { 2.0f,  0.0f },
                { 1.95f, 0.1f },
                { 1.8f,  0.2f },
                { 1.6f,  0.25f },
                { 1.5f,  0.25f },
                { 1.4f,  0.25f },
                { 1.2f,  0.2f },
                { 1.05f, 0.1f },
                { 1.0f,  0.0f },
                { 1.05f,-0.1f },
                { 1.2f, -0.2f },
                { 1.4f, -0.25f },
                { 1.5f, -0.25f },
                { 1.6f, -0.25f },
                { 1.8f, -0.2f },
                { 1.95f,-0.1f },
                { 2.0f,  0.0f }
            };
            mesh_info_t info(coords, RENDER_REVOLUTION);
            auto mesh = std::make_shared<RevolutionMesh>(info, camera);
            mesh->set_position(vec3(0.0f, -2.5f, 0.0f));
            mesh->set_color(0x001F);
            scene.add_node(mesh, ID(TOROIDE));
        }

        // 6) Triangular prism (extrusion)
        {
            vector<coordinates_t> coords = {
                { 0.0f,   0.0f },
                { 1.0f,   0.0f },
                { 0.5f, 0.866f },
                { 0.0f,   0.0f }
            };
            mesh_info_t info(coords, RENDER_EXTRUDE);
            auto mesh = std::make_shared<ExtrudeMesh>(info, camera);
            mesh->set_position(vec3(3.0f, 2.5f, 0.0f));
            mesh->set_scale(vec3(1.0f));
            mesh->set_color(0xFFE0);
            scene.add_node(mesh, ID(PRISMA_TRIANG));
        }

        // 7) Rectangular prism (extrusion)
        {
            vector<coordinates_t> coords = {
                { 0.0f, 0.0f },
                { 1.0f, 0.0f },
                { 1.0f, 2.0f },
                { 0.0f, 2.0f },
                { 0.0f, 0.0f }
            };
            mesh_info_t info(coords, RENDER_EXTRUDE);
            auto mesh = std::make_shared<ExtrudeMesh>(info, camera);
            mesh->set_position(vec3(0.0f, 0.0f, -3.0f));
            mesh->set_scale(vec3(1.0f));
            mesh->set_color(0x07FF);
            scene.add_node(mesh, ID(PRISMA_RECT));
        }

        // 8) Star (extrusion)
        {
            vector<coordinates_t> coords = {
                {  0.0f,     1.0f },
                {  0.2245f,  0.3090f },
                {  0.9511f,  0.3090f },
                {  0.3633f, -0.1180f },
                {  0.5878f, -0.8090f },
                {  0.0f,    -0.3820f },
                { -0.5878f, -0.8090f },
                { -0.3633f, -0.1180f },
                { -0.9511f,  0.3090f },
                { -0.2245f,  0.3090f },
                {  0.0f,     1.0f }
            };
            mesh_info_t info(coords, RENDER_EXTRUDE);
            auto mesh = std::make_shared<ExtrudeMesh>(info, camera);
            mesh->set_position(vec3(0.0f, 0.0f, 3.0f));
            mesh->set_scale(vec3(1.0f));
            mesh->set_color(0xF81F);
            scene.add_node(mesh, ID(ESTRELLA));
        }

        // 9) Letter "C" (extrusion)
        {
            vector<coordinates_t> coords = {
                { 1.0f, 0.0f },
                { 0.5f, 0.0f },
                { 0.0f, 0.5f },
                { 0.0f, 1.5f },
                { 0.5f, 2.0f },
                { 1.0f, 2.0f },
                { 0.5f, 2.0f },
                { 0.0f, 1.5f },
                { 0.0f, 0.5f },
                { 0.5f, 0.0f },
                { 1.0f, 0.0f }
            };
            mesh_info_t info(coords, RENDER_EXTRUDE);
            auto mesh = std::make_shared<ExtrudeMesh>(info, camera);
            mesh->set_position(vec3(-3.0f, -2.5f, 0.0f));
            mesh->set_scale(vec3(1.0f));
            mesh->set_color(0xFBE0);
            scene.add_node(mesh, ID(LETRA_C));
        }

        // 10) Hexagon (extrusion)
        {
            vector<coordinates_t> coords = {
                {  1.000f,  0.000f },
                {  0.500f,  0.866f },
                { -0.500f,  0.866f },
                { -1.000f,  0.000f },
                { -0.500f, -0.866f },
                {  0.500f, -0.866f },
                {  1.000f,  0.000f }
            };
            mesh_info_t info(coords, RENDER_EXTRUDE);
            auto mesh = std::make_shared<ExtrudeMesh>(info, camera);
            mesh->set_position(vec3(3.0f, -2.5f, 0.0f));
            mesh->set_scale(vec3(1.0f));
            mesh->set_color(0xF80F);
            scene.add_node(mesh, ID(HEXAGONO));
        }
    }
}

extern "C" void app_main(void)
{
#ifdef CONFIG_GRAPHICS_PARALLEL_ENABLED
    thread_pool.start();
#endif

    logger.setLogLevel(1);
    logger.Log(MAIN_TAG, 3, "Starting basic_scene example");

    Camera camera(float(SCREEN_W) / SCREEN_H);
    camera.set_location(glm::vec3(0.f, 0.f, -15.f));
    camera.set_target(glm::vec3(0.f, 0.f, 0.f));

    Scene scene(&camera);
    setupScene(scene, camera);

    logger.Log(MAIN_TAG, 3, "Creating renderer (%ux%u)", SCREEN_W, SCREEN_H);
#ifdef CONFIG_IDF_TARGET_ESP32P4
    Ragot::DriverEK79007 display;
#elif CONFIG_IDF_TARGET_ESP32S3
    Ragot::Driver_ST7262 display;
#endif
    Ragot::Renderer renderer(SCREEN_W, SCREEN_H, display);
    renderer.set_scene(&scene);
    renderer.init();

    renderer.set_light(Ragot::DirectionalLight(
        glm::vec3(-0.4f, 0.6f, 0.8f),
        0.50f,
        0.80f
    ));

    scene.start();
    renderer.start();

#ifndef CONFIG_GRAPHICS_PARALLEL_ENABLED
    main_loop(renderer, scene);
#else
    std::this_thread::sleep_until(std::chrono::steady_clock::time_point::max());
#endif
}
