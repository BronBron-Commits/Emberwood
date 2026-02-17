# Install script for directory: C:/Projects/Emberwood/vcpkg_installed/vcpkg/blds/sdl2-ttf/src/ase-2.24.0-bdb8d0cb53.clean

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Projects/Emberwood/vcpkg_installed/vcpkg/pkgs/sdl2-ttf_x64-windows")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "OFF")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "C:/Projects/Emberwood/vcpkg_installed/vcpkg/blds/sdl2-ttf/x64-windows-rel/SDL2_ttf.lib")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "library" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "C:/Projects/Emberwood/vcpkg_installed/vcpkg/blds/sdl2-ttf/x64-windows-rel/SDL2_ttf.dll")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/SDL2" TYPE FILE FILES "C:/Projects/Emberwood/vcpkg_installed/vcpkg/blds/sdl2-ttf/src/ase-2.24.0-bdb8d0cb53.clean/SDL_ttf.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake" TYPE FILE FILES
    "C:/Projects/Emberwood/vcpkg_installed/vcpkg/blds/sdl2-ttf/x64-windows-rel/SDL2_ttfConfig.cmake"
    "C:/Projects/Emberwood/vcpkg_installed/vcpkg/blds/sdl2-ttf/x64-windows-rel/SDL2_ttfConfigVersion.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "devel" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cmake/SDL2_ttf-shared-targets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cmake/SDL2_ttf-shared-targets.cmake"
         "C:/Projects/Emberwood/vcpkg_installed/vcpkg/blds/sdl2-ttf/x64-windows-rel/CMakeFiles/Export/272ceadb8458515b2ae4b5630a6029cc/SDL2_ttf-shared-targets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cmake/SDL2_ttf-shared-targets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cmake/SDL2_ttf-shared-targets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake" TYPE FILE FILES "C:/Projects/Emberwood/vcpkg_installed/vcpkg/blds/sdl2-ttf/x64-windows-rel/CMakeFiles/Export/272ceadb8458515b2ae4b5630a6029cc/SDL2_ttf-shared-targets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake" TYPE FILE FILES "C:/Projects/Emberwood/vcpkg_installed/vcpkg/blds/sdl2-ttf/x64-windows-rel/CMakeFiles/Export/272ceadb8458515b2ae4b5630a6029cc/SDL2_ttf-shared-targets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "devel" OR NOT CMAKE_INSTALL_COMPONENT)
  
        # FIXME: use file(COPY_FILE) if CMake 3.21+
        execute_process(COMMAND "C:/Program Files/CMake/bin/cmake.exe" -E copy_if_different
            "C:/Projects/Emberwood/vcpkg_installed/vcpkg/blds/sdl2-ttf/x64-windows-rel/SDL2_ttf-Release.pc"
            "C:/Projects/Emberwood/vcpkg_installed/vcpkg/blds/sdl2-ttf/x64-windows-rel/SDL2_ttf.pc")
        file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig"
            TYPE FILE
            FILES "C:/Projects/Emberwood/vcpkg_installed/vcpkg/blds/sdl2-ttf/x64-windows-rel/SDL2_ttf.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "library" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/licenses/SDL2_ttf" TYPE FILE FILES "C:/Projects/Emberwood/vcpkg_installed/vcpkg/blds/sdl2-ttf/src/ase-2.24.0-bdb8d0cb53.clean/LICENSE.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/Projects/Emberwood/vcpkg_installed/vcpkg/blds/sdl2-ttf/x64-windows-rel/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
if(CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_COMPONENT MATCHES "^[a-zA-Z0-9_.+-]+$")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
  else()
    string(MD5 CMAKE_INST_COMP_HASH "${CMAKE_INSTALL_COMPONENT}")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INST_COMP_HASH}.txt")
    unset(CMAKE_INST_COMP_HASH)
  endif()
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/Projects/Emberwood/vcpkg_installed/vcpkg/blds/sdl2-ttf/x64-windows-rel/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
