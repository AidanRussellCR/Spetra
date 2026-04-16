add_library(spetra STATIC
    src/game.cpp
    src/window.cpp
    src/color.cpp
    src/input.cpp
    src/timer.cpp
    src/scene_manager.cpp
    src/texture.cpp
    src/filesystem.cpp
)

target_include_directories(spetra
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3)
find_package(SDL3_image REQUIRED CONFIG)

target_link_libraries(spetra
    PUBLIC
        SDL3::SDL3
        SDL3_image::SDL3_image
)

target_compile_features(spetra PUBLIC cxx_std_20)
