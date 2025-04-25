#include "Renderer.hpp"
#include <vector>
#include "CommonTypes.hpp"
#include "Scene.hpp"
#include "RevolutionMesh.hpp"
#include "ExtrudeMesh.hpp"
#include "Id.hpp"
#include "Logger.hpp"


static const char* MAIN_TAG = "Main";

using namespace std;
using namespace Ragot;

#if ESP_PLATFORM == 1
extern "C" void app_main(void)
#else
int main()
#endif
{
    logger.setLogLevel (0); // 0 NONE, 1 ERROR, 2 WARNING, 3 INFO

    logger.Log (MAIN_TAG, 3, "Iniciando aplicación");
    // logger.Log (MAIN_TAG, 3, "Memoria libre inicial: %u bytes", esp_get_free_heap_size());
    
    logger.Log (MAIN_TAG, 3, "Creando renderer (600x1024)");
    Ragot::Renderer renderer(1024, 600);
    
    uint32_t frame_count = 0;
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
                                             { 0 ,  0 } , { 1 ,  1 },
                                             { 2 ,  1 } , { 3 ,  0 },
                                             { 2 , -1 } , { 1 , -1 } };

    mesh_info_t mesh_info_cup  (coords, RENDER_REVOLUTION);
    mesh_info_t mesh_info_cube (coords_cube, RENDER_EXTRUDE);

    // RevolutionMesh mesh_cup  (mesh_info_cup );
    ExtrudeMesh    mesh_cube (mesh_info_cube, camera);

    // mesh_cup.set_position  (glm::vec3{ 0.f, 0.f, -5.f });
    mesh_cube.set_position (glm::vec3{ 0.f, 10.f, 0.f });
    mesh_cube.set_scale    (glm::vec3{ 0.5f, 0.5f, 0.5f });

    // scene.add_node(&mesh_cup , ID(COPA));
    scene.add_node(&mesh_cube, ID(CUBO));

    renderer.set_scene(&scene);
    renderer.init();

    bool update = true;

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
        // ESP_LOGI(MAIN_TAG, "Esperando 50ms");
        // vTaskDelay(pdMS_TO_TICKS(50));
        
        frame_count++;
        
        // Log periódico del estado de memoria
        if (frame_count % 100 == 0) 
        {
            logger.Log (MAIN_TAG, 3, "Estado después de %u frames:", frame_count);
            // ESP_LOGI(MAIN_TAG, "Memoria libre: %u bytes", esp_get_free_heap_size());
        }
    }
}
