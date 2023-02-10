add_executable(
    main
    main.cpp
    include/Test.h
)

set_property(TARGET main PROPERTY CXX_STANDARD 17)


target_include_directories(
    main
    PRIVATE ${CMAKE_SOURCE_DIR}/src/include
)