find_package(nlohmann_json REQUIRED)

add_executable(demo
    src/main.cpp
    src/map_scene.cpp
    src/map_loader.cpp
    src/map_data.cpp
    src/draw_order.cpp
    src/condition_eval.cpp
    src/event_runner.cpp
    src/pause_overlay_scene.cpp
    src/battle_stub_scene.cpp
    src/menu_scene.cpp
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

# Tests; the test links only the map code + nlohmann (no SDL/engine)
enable_testing()

add_executable(draw_order_test
    tests/draw_order_test.cpp
    src/draw_order.cpp
)

target_include_directories(draw_order_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

add_test(NAME draw_order COMMAND draw_order_test)

# Event system
add_executable(event_test
    tests/event_test.cpp
    src/event_runner.cpp
    src/condition_eval.cpp
)

target_include_directories(event_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(event_test
    PRIVATE
        spetra
        nlohmann_json::nlohmann_json
)

add_test(NAME events COMMAND event_test)
