
include_directories(include)

include_directories(${CMAKE_BINARY_DIR}/src/core)

add_subdirectory(api)
add_subdirectory(src)

