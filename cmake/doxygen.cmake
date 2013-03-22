
### Setup doxygen documentation 
find_package(Doxygen)
if(DOXYGEN_FOUND)
# Using a .in file means we can use CMake to insert project settings
# into the doxyfile.  For example, CMake will replace @PROJECT_NAME@ in
# a configured file with the CMake PROJECT_NAME variable's value.

set(FAIL_DOC_OUTPUT "${PROJECT_BINARY_DIR}/doc")
set(FAIL_DOC_EXCLUDE  "${PROJECT_SOURCE_DIR}/simulators ${PROJECT_SOURCE_DIR}/build ${PROJECT_SOURCE_DIR}/src/core/util/pstream.h")
file(MAKE_DIRECTORY ${FAIL_DOC_OUTPUT})

configure_file(${PROJECT_SOURCE_DIR}/cmake/Doxyfile.in
		${PROJECT_BINARY_DIR}/Doxyfile @ONLY}
)

## call make doc to generate documentation
add_custom_target(doc
		${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
		DEPENDS ${PROJECT_BINARY_DIR}/Doxyfile
		WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
		COMMENT "[${PROJECT_NAME}] Generating Fail* documentation with Doxygen" VERBATIM
)
endif(DOXYGEN_FOUND)

