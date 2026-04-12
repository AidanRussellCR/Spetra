add_library(spetra STATIC
    src/game.cpp
    src/window.cpp
    src/color.cpp
)

target_include_directories(spetra
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3)

target_link_libraries(spetra
    PUBLIC
        SDL3::SDL3
)

target_compile_features(spetra PUBLIC cxx_std_20)
