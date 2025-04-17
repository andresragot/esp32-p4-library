idf_component_register(SRCS "main.cpp"
                            "Scene.cpp" 
                            "Camera.cpp" 
                            "Assets.cpp"
                            "ExtrudeMesh.cpp"
                            "Framebuffer.cpp"
                            "Mesh.cpp"
                            "MeshSerializer.cpp"
                            "Rasterizer.cpp"
                            "Renderer.cpp"
                            "RevolutionMesh.cpp"
                            "driver_ek79007.cpp" 
                    INCLUDE_DIRS "." "libraries/glm/include"
                    REQUIRES driver
                    REQUIRES esp_lcd
                    REQUIRES esp_timer
                    )
