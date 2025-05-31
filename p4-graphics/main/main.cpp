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
    unsigned frame_count = 0;
    size_t ram_usage = 0;

    std::chrono::high_resolution_clock::time_point last_tick = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point current_tick;
    std::chrono::high_resolution_clock::duration elapsed_time;

    while (true)
    {
        current_tick = std::chrono::high_resolution_clock::now();
        logger.Log (MAIN_TAG, 3, "\n--- Frame %u ---", frame_count);
        if (update)
        {
#ifndef CONFIG_GRAPHICS_PARALLEL_ENABLED
            scene.update(elapsed_time.count());
#endif
            // update = false;
        }
        else
        {
            update = true;
        }
#ifndef CONFIG_GRAPHICS_PARALLEL_ENABLED            
        renderer.render ();
#endif
        
        frame_count++;

        elapsed_time = current_tick - last_tick;
        last_tick = current_tick;
        ram_usage = esp_get_free_heap_size();
        logger.Log (MAIN_TAG, 1, "Tiempo transcurrido: %.6f segundos\n", std::chrono::duration<float>(elapsed_time).count());
        logger.Log (MAIN_TAG, 1, "Frames renderizados: %u\n", frame_count);
        logger.Log (MAIN_TAG, 1, "FPS: %.2f\n", 1.f / std::chrono::duration<float>(elapsed_time).count());
        logger.Log (MAIN_TAG, 1, "Uso de RAM: %zu bytes\n", ram_usage);
        
        // Log periódico del estado de memoria
        if (frame_count % 100 == 0)
        {
            logger.Log (MAIN_TAG, 3, "Estado después de %u frames:", frame_count);
        }
        // std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Simula un frame rate de 10 FPS
    }
}
#else
void main_loop (Renderer & renderer, Scene & scene, Window & window)
{
    glViewport (0, 0, 1024, 600);
    
    static bool update = true;

    bool exit = false;
    
    std::chrono::high_resolution_clock::time_point last_tick = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point current_tick;
    std::chrono::high_resolution_clock::duration elapsed_time;
    
    while (!exit)
    {
        current_tick = std::chrono::high_resolution_clock::now();
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
        
        last_tick = current_tick;
    }
}
#endif


namespace
{
    void setupScene(Ragot::Scene& scene, Ragot::Camera& camera)
    {
        using namespace Ragot;
        using namespace glm;

        // ---------------------------------------------------
        // 1) CILINDRO (revolución de un rectángulo vertical)
        // ---------------------------------------------------
        {
            vector<coordinates_t> coords_cilindro = {
                { 1.0f, 0.0f },
                { 1.0f, 2.0f },
                { 0.0f, 2.0f }
            };
            mesh_info_t mesh_info_cilindro(coords_cilindro, RENDER_REVOLUTION);
            std::shared_ptr < RevolutionMesh > mesh_cilindro = std::make_shared < RevolutionMesh >(mesh_info_cilindro, camera);
            mesh_cilindro->set_position(vec3( 0.0f, 0.0f,  0.0f));  // centro de la escena
            mesh_cilindro->set_color (0x0000);
            // scene.add_node(mesh_cilindro, ID(CILINDRO));
        }

        // ---------------------------------------------------
        // 2) CONO (revolución de un triángulo simple)
        // ---------------------------------------------------
        {
            vector<coordinates_t> coords_cono = {
                { 1.0f, 0.0f },
                { 0.0f, 2.0f },
                { 0.0f, 0.0f }
            };
            mesh_info_t mesh_info_cono(coords_cono, RENDER_REVOLUTION);
            std::shared_ptr < RevolutionMesh > mesh_cono = std::make_shared < RevolutionMesh > (mesh_info_cono, camera);
            mesh_cono->set_position(vec3( 3.0f, 0.0f,  0.0f));  // a la derecha del cilindro
            mesh_cono->set_color (0x8410); // Color cono (gris)
            // scene.add_node(mesh_cono, ID(CONO));
        }

        // ---------------------------------------------------
        // 3) “VASO” (revolución de un perfil tipo copa)
        // ---------------------------------------------------
        {
            vector<coordinates_t> coords_vaso = {
                { 0.0f, 0.0f },
                { 2.0f, 0.0f },
                { 2.0f, 3.0f },
                { 1.5f, 3.5f },
                { 1.0f, 4.0f },
                { 0.8f, 4.2f },
                { 0.0f, 4.2f }
            };
            mesh_info_t mesh_info_vaso(coords_vaso, RENDER_REVOLUTION);
            std::shared_ptr < RevolutionMesh > mesh_vaso = std::make_shared < RevolutionMesh >(mesh_info_vaso, camera);
            mesh_vaso->set_position(vec3(-3.0f, 0.0f,  0.0f));  // a la izquierda del cilindro
            mesh_vaso->set_color (0xF800); // Color vaso (Rojo)
            // scene.add_node(mesh_vaso, ID(VASO));
        }

        // ---------------------------------------------------
        // 4) SEMIESFERA (revolución de un semicírculo)
        // ---------------------------------------------------
        {
            vector<coordinates_t> coords_semiesfera = {
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
            mesh_info_t mesh_info_semiesf(coords_semiesfera, RENDER_REVOLUTION);
            std::shared_ptr < RevolutionMesh > mesh_semiesfera = std::make_shared < RevolutionMesh > (mesh_info_semiesf, camera);
            mesh_semiesfera->set_position(vec3( 0.0f, 2.5f,  0.0f)); // encima del cilindro/cono/vaso
            mesh_semiesfera-> set_color (0x07F0); // Color semiesfera (Verde claro)
            // scene.add_node(mesh_semiesfera, ID(SEMIESFERA));
        }

        // ---------------------------------------------------
        // 5) TOROIDE (revolución de un círculo desplazado)
        // ---------------------------------------------------
        {
            vector<coordinates_t> coords_toroide = {
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
            mesh_info_t mesh_info_toroide(coords_toroide, RENDER_REVOLUTION);
            std::shared_ptr < RevolutionMesh > mesh_toroide = std::make_shared < RevolutionMesh > (mesh_info_toroide, camera);
            mesh_toroide->set_position(vec3( 0.0f, -2.5f,  0.0f)); // debajo del cilindro/cono/vaso
            mesh_toroide->set_color (0x001F); // Color toroide (Azul oscuro)
            scene.add_node(mesh_toroide, ID(TOROIDE));
        }

        // ---------------------------------------------------
        // 6) PRISMA TRIANGULAR (extrusión de triángulo equilátero)
        // ---------------------------------------------------
        {
            vector<coordinates_t> coords_prisma_triangulo = {
                { 0.0f,   0.0f },
                { 1.0f,   0.0f },
                { 0.5f, 0.866f },
                { 0.0f,   0.0f }
            };
            mesh_info_t mesh_info_prisma_triang(coords_prisma_triangulo, RENDER_EXTRUDE);
            std::shared_ptr < ExtrudeMesh > mesh_prisma_triang = std::make_shared < ExtrudeMesh > (mesh_info_prisma_triang, camera);
            mesh_prisma_triang->set_position(vec3( 3.0f, 2.5f,  0.0f)); // esquina superior derecha
            mesh_prisma_triang->set_scale(vec3(1.0f));                // tamaño base 1

            mesh_prisma_triang->set_color (0xFFE0); // Color prisma triangular (Amarillo claro)
            // scene.add_node(mesh_prisma_triang, ID(PRISMA_TRIANG));
        }

        // ---------------------------------------------------
        // 7) PRISMA RECTANGULAR (extrusión de rectángulo)
        // ---------------------------------------------------
        {
            vector<coordinates_t> coords_prisma_rect = {
                { 0.0f,  0.0f },
                { 1.0f,  0.0f },
                { 1.0f,  2.0f },
                { 0.0f,  2.0f },
                { 0.0f,  0.0f }
            };
            mesh_info_t mesh_info_prisma_rect(coords_prisma_rect, RENDER_EXTRUDE);
            std::shared_ptr < ExtrudeMesh > mesh_prisma_rect = std::make_shared < ExtrudeMesh > (mesh_info_prisma_rect, camera);
            mesh_prisma_rect->set_position(vec3( 0.0f,  0.0f, -3.0f)); // “atrás” de la fila principal
            mesh_prisma_rect->set_scale(vec3(1.0f));                   // escala base
            mesh_prisma_rect->set_color (0x07FF); // Color prisma rectangular (Cian claro)
            // scene.add_node(mesh_prisma_rect, ID(PRISMA_RECT));
        }

        // ---------------------------------------------------
        // 8) ESTRELLA (extrusión de estrella de 5 puntas)
        // ---------------------------------------------------
        {
            vector<coordinates_t> coords_estrella = {
                {  0.0f,  1.0f },
                {  0.2245f, 0.3090f },
                {  0.9511f, 0.3090f },
                {  0.3633f, -0.1180f },
                {  0.5878f, -0.8090f },
                {  0.0f,   -0.3820f },
                { -0.5878f, -0.8090f },
                { -0.3633f, -0.1180f },
                { -0.9511f,  0.3090f },
                { -0.2245f,  0.3090f },
                {  0.0f,     1.0f }
            };
            mesh_info_t mesh_info_estrella(coords_estrella, RENDER_EXTRUDE);
            std::shared_ptr < ExtrudeMesh > mesh_estrella = std::make_shared < ExtrudeMesh > (mesh_info_estrella, camera);
            mesh_estrella->set_position(vec3( 0.0f,  0.0f,  3.0f)); // “adelante” de la fila principal
            mesh_estrella->set_scale(vec3(1.0f));
            mesh_estrella->set_color (0xF81F); // Color estrella (Magenta)
            // scene.add_node(mesh_estrella, ID(ESTRELLA));
        }

        // ---------------------------------------------------
        // 9) LETRA “C” (extrusión de silueta de letra)
        // ---------------------------------------------------
        {
            vector<coordinates_t> coords_letra_c = {
                { 1.0f,  0.0f },
                { 0.5f,  0.0f },
                { 0.0f,  0.5f },
                { 0.0f,  1.5f },
                { 0.5f,  2.0f },
                { 1.0f,  2.0f },
                { 0.5f,  2.0f },
                { 0.0f,  1.5f },
                { 0.0f,  0.5f },
                { 0.5f,  0.0f },
                { 1.0f,  0.0f }
            };
            mesh_info_t mesh_info_letra_c(coords_letra_c, RENDER_EXTRUDE);
            std::shared_ptr < ExtrudeMesh > mesh_letra_c = std::make_shared < ExtrudeMesh > (mesh_info_letra_c, camera);
            mesh_letra_c->set_position(vec3(-3.0f, -2.5f,  0.0f)); // esquina inferior izquierda
            mesh_letra_c->set_scale(vec3(1.0f));
            mesh_letra_c->set_color (0xFBE0); // Color letra C (Amarillo claro)
            // scene.add_node(mesh_letra_c, ID(LETRA_C));
        }

        // ---------------------------------------------------
        // 10) HEXÁGONO (extrusión de hexágono regular)
        // ---------------------------------------------------
        {
            vector<coordinates_t> coords_hexagono = {
                { 1.000f,  0.000f },
                { 0.500f,  0.866f },
                { -0.500f, 0.866f },
                { -1.000f, 0.000f },
                { -0.500f, -0.866f },
                { 0.500f,  -0.866f },
                { 1.000f,  0.000f }
            };
            mesh_info_t mesh_info_hexagono(coords_hexagono, RENDER_EXTRUDE);
            std::shared_ptr < ExtrudeMesh > mesh_hexagono = std::make_shared < ExtrudeMesh > (mesh_info_hexagono, camera);
            mesh_hexagono->set_position(vec3( 3.0f, -2.5f,  0.0f)); // esquina inferior derecha
            mesh_hexagono->set_scale(vec3(1.0f));
            mesh_hexagono->set_color (0xF80F); // Color hexágono (Purpura)
            // scene.add_node(mesh_hexagono, ID(HEXAGONO));
        }
    }

}

#if ESP_PLATFORM == 1
extern "C" void app_main(void)
#else
int main(int argc, char * argv[])
#endif
{
#ifdef CONFIG_GRAPHICS_PARALLEL_ENABLED
    thread_pool.start();
#endif
    
    logger.setLogLevel (1); // 0 NONE, 1 ERROR, 2 WARNING, 3 INFO

    logger.Log (MAIN_TAG, 3, "Iniciando aplicación");
    // logger.Log (MAIN_TAG, 3, "Memoria libre inicial: %u bytes", esp_get_free_heap_size());
    
    logger.Log (MAIN_TAG, 3, "Iniciando Window");
#if ESP_PLATFORM != 1
    assets.initialize(argv[0]);

    Ragot::Window window ("P4-Test", Ragot::Window::Position::CENTERED, Ragot::Window::Position::CENTERED, 1024, 600, { 3, 3 });
#endif
    
    logger.Log (MAIN_TAG, 3, "Entrando en bucle de renderizado");

    Camera camera(float (320) / 240.f);
    camera.set_location(glm::vec3(0.f, 0.f, -15.f));
    camera.set_target(glm::vec3(0.f, 0.f, 0.f));

    Scene scene (&camera);

    // vector < coordinates_t > coords = { { 0 ,  0 }, { 4 ,  0 },
    //                                     { 2 ,  2 }, { 2 ,  5 },
    //                                     { 4 ,  7 }, { 6 ,  9 },
    //                                     { 0 ,  9 } };

    // vector < coordinates_t > coords_cube = { { 0 ,  0 } , { 1 ,  1 },
    //                                          { 2 ,  1 } , { 3 ,  0 },
    //                                          { 2 , -1 } , { 1 , -1 },
    //                                          { 0 ,  0 } };

    // mesh_info_t mesh_info_cup  (coords, RENDER_REVOLUTION);
    // mesh_info_t mesh_info_cube (coords_cube, RENDER_EXTRUDE);

    // RevolutionMesh mesh_cup  (mesh_info_cup, camera );
    // ExtrudeMesh    mesh_cube (mesh_info_cube, camera);

    // mesh_cup.set_position  (glm::vec3{ 0.f, 8.f, -0.f });
    // mesh_cube.set_position (glm::vec3{ 0.f, 0.f, 0.f });
    // // mesh_cup.set_scale    (glm::vec3{ 0.1f, 0.1f, 0.1f });

    // scene.add_node(&mesh_cup , ID(COPA));
    // scene.add_node(&mesh_cube, ID(CUBO));
    
    setupScene(scene, camera);

    logger.Log (MAIN_TAG, 3, "Creando renderer (600x1024)");
    Ragot::Renderer renderer(320, 240);
    renderer.set_scene(&scene);
    renderer.init();

    scene.start ();
    renderer.start ();

    
#if ESP_PLATFORM == 1
#ifndef CONFIG_GRAPHICS_PARALLEL_ENABLED
    main_loop(renderer, scene);
#else
    std::this_thread::sleep_until(std::chrono::steady_clock::time_point::max());
#endif
#else
    main_loop(renderer, scene, window);
#endif

}
