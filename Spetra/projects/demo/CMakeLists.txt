add_executable(demo
    src/main.cpp
)

target_link_libraries(demo
    PRIVATE
        spetra
)
