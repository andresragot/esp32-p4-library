#include "Renderer.hpp"
#include <vector>
#include "CommonTypes.hpp"
#include "Scene.hpp"
#include "RevolutionMesh.hpp"
#include "ExtrudeMesh.hpp"
#include "Id.hpp"
#include "Logger.hpp"
#include "Thread_Pool.hpp"

#if ESP_PLATFORM != 1
#include "Window.hpp"
#endif

// ESP Rasterization took 0.0789344  seconds.
// MAC Rasterization took 0.00564271 seconds.

// ESP Rasterization took 0.0787937 seconds.
// MAC Rasterization took 0.00561047 seconds.




static const char* MAIN_TAG = "Main";

using namespace std;
using namespace Ragot;

#if ESP_PLATFORM == 1
void main_loop (Renderer & renderer, Scene & scene)
{
    bool update = true;
    unsigned frame_count;

    while (true)
    {
        logger.Log (MAIN_TAG, 3, "\n--- Frame %u ---", frame_count);
        // renderer.render_debug();
        if (update)
        {
            scene.update(0);
            // update = false;
        }
        else
        {
            update = true;
        }
        renderer.render();
        
        frame_count++;
        
        // Log periódico del estado de memoria
        if (frame_count % 100 == 0)
        {
            logger.Log (MAIN_TAG, 3, "Estado después de %u frames:", frame_count);
        }
    }
}
#else
void main_loop (Renderer & renderer, Scene & scene, Window & window)
{
    glViewport (0, 0, 1024, 600);

    bool exit = false;
    
    while (!exit)
    {
        SDL_Event event;
        
        while (SDL_PollEvent(&event) > 0)
        {
            if (event.type == SDL_QUIT)
            {
                exit = true;
            }
        }
//        scene.update(0);
        
        renderer.render();
        
        window.swap_buffers();
        
    }
}
#endif


#if ESP_PLATFORM == 1
extern "C" void app_main(void)
#else
int main(int argc, char * argv[])
#endif
{
    thread_pool.start();
    
    logger.setLogLevel (1); // 0 NONE, 1 ERROR, 2 WARNING, 3 INFO

    logger.Log (MAIN_TAG, 3, "Iniciando aplicación");
    // logger.Log (MAIN_TAG, 3, "Memoria libre inicial: %u bytes", esp_get_free_heap_size());
    
    logger.Log (MAIN_TAG, 3, "Iniciando Window");
#if ESP_PLATFORM != 1
    Ragot::Window window ("P4-Test", Ragot::Window::Position::CENTERED, Ragot::Window::Position::CENTERED, 1024, 600, { 3, 3 });
#endif
    
    logger.Log (MAIN_TAG, 3, "Creando renderer (600x1024)");
    Ragot::Renderer renderer(1024, 600);
    
    logger.Log (MAIN_TAG, 3, "Entrando en bucle de renderizado");

    Camera camera(float (1024) / 600.f);
    camera.set_location(glm::vec3(0.f, 0.f, -15.f));
    camera.set_target(glm::vec3(0.f, 0.f, 0.f));

    Scene scene (&camera);

    vector < coordinates_t > coords = { { 0 ,  0 }, { 4 ,  0 },
                                        { 2 ,  2 }, { 2 ,  5 },
                                        { 4 ,  7 }, { 6 ,  9 },
                                        { 0 ,  9 } };

    vector < coordinates_t > coords_cube = { { 0 ,  0 } , { 1 ,  1 },
                                             { 2 ,  1 } , { 3 ,  0 },
                                             { 2 , -1 } , { 1 , -1 },
                                             { 0 ,  0 } };

    mesh_info_t mesh_info_cup  (coords, RENDER_REVOLUTION);
    mesh_info_t mesh_info_cube (coords_cube, RENDER_EXTRUDE);

    RevolutionMesh mesh_cup  (mesh_info_cup, camera );
    ExtrudeMesh    mesh_cube (mesh_info_cube, camera);

    mesh_cup.set_position  (glm::vec3{ 0.f, 8.f, -0.f });
    mesh_cube.set_position (glm::vec3{ 0.f, 0.f, 0.f });
    // mesh_cup.set_scale    (glm::vec3{ 0.1f, 0.1f, 0.1f });

    scene.add_node(&mesh_cup , ID(COPA));
    scene.add_node(&mesh_cube, ID(CUBO));

    renderer.set_scene(&scene);
    renderer.init();
    
#if ESP_PLATFORM == 1
    main_loop(renderer, scene);
#else
    main_loop(renderer, scene, window);
#endif

}
