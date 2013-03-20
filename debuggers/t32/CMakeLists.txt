# T32 remote specific configuration options
OPTION( T32_MOCK_API "Intercept remote calls to T32 for debugging" OFF)

set(T32_PORTNUM 20000 CACHE PATH "TCP Port number for remote access.")
set(T32_PACKLEN 1024 CACHE INTERNAL "TCP Packet size, max. 1024")
configure_file( include/t32config.hpp.in ${CMAKE_BINARY_DIR}/include/t32config.hpp)

include_directories(include ${CMAKE_BINARY_DIR}/include)

include_directories(${CMAKE_BINARY_DIR}/src/core)

  set(T32_ARCHITECTURE armm3 CACHE PATH "Setup target architecture for default cmm scripts (currently only armm3)")

add_subdirectory(api)
add_subdirectory(src)
add_subdirectory(sim)
