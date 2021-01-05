#### C++14 ####
# We need at least C++11, as some library headers begin to require it.  C++14
# has already aged sufficiently to mandate it here.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

##### Verbose make ####
option( VERBOSE_MAKE "Verbose Makefile output" OFF) # defaults to OFF
	set(CMAKE_VERBOSE_MAKEFILE ${VERBOSE_MAKE})

### Additional compiler and linker flags ##
# -Wunused-local-typedefs is included in -Wall since GCC 4.8, and generates a
# flood of "typedef '...' locally defined but not used" warnings in
# ac++-1.2-generated code
set(CMAKE_C_FLAGS "-g -Wall -Wno-unused-local-typedefs")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,-gc-sections")

set(CMAKE_C_COMPILER "gcc")
find_program(AGXX ag++ PATHS /usr/bin /usr/local/bin /opt/bin /opt/local/bin ENV PATH)
if(${AGXX} MATCHES "NOTFOUND")
  message(FATAL_ERROR "ag++ not found.")
endif()
set(CMAKE_CXX_COMPILER "${AGXX}")
set(CMAKE_AGPP_FLAGS "-D__NO_MATH_INLINES" CACHE STRING "Additional ag++ flags (space-separated), e.g., --keep_woven")
  ## Here we add the build dir holding the generated header files (protobuf)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --Xweaver -p ${PROJECT_SOURCE_DIR}/src -p ${PROJECT_SOURCE_DIR}/simulators -p ${PROJECT_SOURCE_DIR}/debuggers -p ${PROJECT_SOURCE_DIR}/tools -p ${PROJECT_BINARY_DIR}/src ${CMAKE_AGPP_FLAGS} --Xcompiler")

add_definitions(-D_FILE_OFFSET_BITS=64)

message(STATUS "[${PROJECT_NAME}] Compiler: ${CMAKE_C_COMPILER} + ${CMAKE_CXX_COMPILER}" )
