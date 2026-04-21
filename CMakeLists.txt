idf_component_register(
    SRCS "src/Renderer.cpp"
         "src/Scene.cpp"
         "src/Camera.cpp"
         "src/Mesh.cpp"
         "src/ExtrudeMesh.cpp"
         "src/RevolutionMesh.cpp"
         "src/Rasterizer.cpp"
         "src/FrameBuffer.cpp"
         "src/Logger.cpp"
         "src/Thread_Pool.cpp"
         "src/atomic_stubs.c"
    INCLUDE_DIRS "include"
    REQUIRES andresragot__driver_lcd glm esp_timer freertos
)