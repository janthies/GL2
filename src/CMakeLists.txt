add_executable(
    main
    main.cpp
    include/Test.h
    include/Pch.h
)

set_property(TARGET main PROPERTY CXX_STANDARD 17)


target_link_libraries(
    main
    PRIVATE ${OPENGL_LIBRARY}
    PRIVATE ${CMAKE_SOURCE_DIR}/external/glew/lib/Release/x64/glew32s.lib
    PRIVATE glfw
    PRIVATE glm
)

target_include_directories(
    main
    PRIVATE ${CMAKE_SOURCE_DIR}/src/include
    PRIVATE ${CMAKE_SOURCE_DIR}/external/glew/include
)

target_precompile_headers(
    main
    PRIVATE ${CMAKE_SOURCE_DIR}/src/include/Pch.h
)