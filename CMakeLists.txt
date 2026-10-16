cmake_minimum_required(VERSION 3.24)
project(TP1_JeuVideo LANGUAGES CXX)

# gl.lib fourni est compilé avec /MT (LIBCMT), y compris pour nos builds Debug.
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()

# build/ contient les dépendances fournies; out/ contient les résultats.
add_library(dependances INTERFACE)
target_include_directories(dependances INTERFACE
    "${PROJECT_SOURCE_DIR}/build/include"
    "${PROJECT_SOURCE_DIR}/external/glm")
target_compile_features(dependances INTERFACE cxx_std_17)
if(WIN32)
    if(NOT MSVC OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR "Les binaires fournis nécessitent MSVC x64.")
    endif()
    foreach(name IN ITEMS SDL3 SDL3_image SDL3_ttf gl)
        add_library(${name}_fourni UNKNOWN IMPORTED)
        set_target_properties(${name}_fourni PROPERTIES
            IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/build/lib/${name}.lib")
        target_link_libraries(dependances INTERFACE ${name}_fourni)
    endforeach()
    target_link_libraries(dependances INTERFACE opengl32)
elseif(APPLE)
    find_package(SDL3 CONFIG REQUIRED)
    find_package(SDL3_image CONFIG REQUIRED)
    find_package(SDL3_ttf CONFIG REQUIRED)
    find_library(OPENGL_FRAMEWORK OpenGL REQUIRED)
    target_link_libraries(dependances INTERFACE
        SDL3::SDL3 SDL3_image::SDL3_image SDL3_ttf::SDL3_ttf
        "${PROJECT_SOURCE_DIR}/build/lib/libglad.a" "${OPENGL_FRAMEWORK}")
else()
    message(FATAL_ERROR "Ce squelette fournit des binaires Windows et macOS.")
endif()

add_library(moteur STATIC
    src/moteur/Bibliotheques.cpp
    src/moteur/Engine.cpp
    src/moteur/Window.cpp
    src/moteur/Shader.cpp
    src/moteur/Program.cpp
    src/moteur/VAO.cpp)
target_include_directories(moteur PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(moteur PUBLIC dependances)
set_target_properties(moteur PROPERTIES CXX_EXTENSIONS OFF)
add_executable(program src/main.cpp)
target_link_libraries(program PRIVATE moteur)
target_compile_definitions(program PRIVATE MOTEUR_SHADER_DIR="${PROJECT_SOURCE_DIR}/assets/shaders")
set_target_properties(program PROPERTIES CXX_EXTENSIONS OFF)
if(MSVC)
    target_compile_options(moteur PRIVATE /W4 /utf-8)
    target_compile_options(program PRIVATE /W4 /utf-8)
endif()
if(WIN32)
    foreach(name IN ITEMS SDL3 SDL3_image SDL3_ttf)
        add_custom_command(TARGET program POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${PROJECT_SOURCE_DIR}/build/${name}.dll"
                "$<TARGET_FILE_DIR:program>" VERBATIM)
    endforeach()
endif()
enable_testing()
add_test(NAME bibliotheques COMMAND program --check)
