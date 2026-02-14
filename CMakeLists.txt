cmake_minimum_required(VERSION 3.10)
project(Emberwood)

set(CMAKE_CXX_STANDARD 17)


add_executable(Emberwood
    src/main.cpp
    src/engine/Renderer.cpp
    src/engine/TextureManager.cpp
    src/entities/Tree.cpp
)

# Link SDL2 libraries (adjust path if needed)
target_link_libraries(Emberwood PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/SDL2.lib
    ${CMAKE_SOURCE_DIR}/lib/SDL2main.lib
)

include_directories(include)
