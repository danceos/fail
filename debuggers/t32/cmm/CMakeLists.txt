
## Setup T32 target architecture for startup scripts
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/cmm)
configure_file(t32.cmm ${PROJECT_BINARY_DIR}/cmm/t32.cmm COPYONLY)

set(T32_ARCHITECTURE armm3 CACHE PATH "Setup target architecture for default cmm scripts (currently only armm3)")
message(STATUS "[FAIL*] T32 Architecture: ${T32_ARCHITECTURE}")
add_subdirectory(${T32_ARCHITECTURE})

message(STATUS "[FAIL*] T32 CPU name: ${T32_CPUNAME}")

