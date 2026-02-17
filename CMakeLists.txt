cmake_minimum_required(VERSION 3.10)




project(Emberwood)

set(CMAKE_CXX_STANDARD 17)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    C:/msys64/mingw64/include/SDL2
)

add_executable(Emberwood
    src/main.cpp
    src/engine/Renderer.cpp
    src/engine/TextureManager.cpp
    src/entities/Tree.cpp
)

target_link_libraries(Emberwood PRIVATE
    C:/msys64/mingw64/lib/libmingw32.a
    C:/msys64/mingw64/lib/libSDL2main.a
    C:/msys64/mingw64/lib/libSDL2.a
    C:/msys64/mingw64/lib/libSDL2_ttf.a
    C:/msys64/mingw64/lib/libfreetype.a
    C:/msys64/mingw64/lib/libharfbuzz.a
    C:/msys64/mingw64/lib/libgraphite2.a
    C:/msys64/mingw64/lib/libpng.a
    C:/msys64/mingw64/lib/libbz2.a
    C:/msys64/mingw64/lib/libz.a
    C:/msys64/mingw64/lib/libbrotlidec.a
    C:/msys64/mingw64/lib/libbrotlicommon.a
    C:/msys64/mingw64/lib/libbrotlienc.a
    -lwinmm
    -lcfgmgr32
    -limm32
    -lversion
    -lsetupapi
    -ldwrite
    -luuid
    -lole32
    -loleaut32
    -lcomdlg32
    -ladvapi32
    -lrpcrt4
)
