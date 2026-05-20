find_package(nlohmann_json REQUIRED)

add_executable(demo
    src/main.cpp
    src/map_scene.cpp
    src/map_loader.cpp
)

target_include_directories(demo
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(demo
    PRIVATE
        spetra
        nlohmann_json::nlohmann_json
)

add_custom_command(TARGET demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -rf
        $<TARGET_FILE_DIR:demo>/assets

    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:demo>/assets
)
