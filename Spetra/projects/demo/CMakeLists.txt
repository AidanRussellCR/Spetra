add_executable(demo
    src/main.cpp
    src/color_scene.cpp
)

target_include_directories(demo
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(demo
    PRIVATE
        spetra
)
