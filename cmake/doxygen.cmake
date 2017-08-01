
### Setup doxygen documentation 
include(CMakeParseArguments) # work around internal FindDoxygen bug
find_package(Doxygen)
if(DOXYGEN_FOUND)
# Using a .in file means we can use CMake to insert project settings
# into the doxyfile.  For example, CMake will replace @PROJECT_NAME@ in
# a configured file with the CMake PROJECT_NAME variable's value.

set(FAIL_DOC_OUTPUT  "${PROJECT_BINARY_DIR}/doc")
execute_process(COMMAND find "${PROJECT_SOURCE_DIR}/src/core/" -type d -printf "%p "
                OUTPUT_VARIABLE FAIL_DOC_SOURCE)
set(FAIL_DOC_SOURCE "${FAIL_DOC_SOURCE} ${PROJECT_SOURCE_DIR}/src/plugins")
set(FAIL_DOC_EXCLUDE_PATTERNS "*/util/pstream.h */util/optionparser/optionparser.h")
file(MAKE_DIRECTORY ${FAIL_DOC_OUTPUT})
# FIXME: The find command does not quote the paths to be processed by doxygen. That
#        means, the path to your FAIL* directory should not contain any blanks.

configure_file(${PROJECT_SOURCE_DIR}/cmake/Doxyfile.in
		${PROJECT_BINARY_DIR}/Doxyfile @ONLY
)

## call make doc to generate documentation
set(line0 "[${PROJECT_NAME}] Generating FAIL* documentation with Doxygen")
set(line1 "       Directories: ${FAIL_DOC_SOURCE}")
set(line2 "       Excluded patterns: ${FAIL_DOC_EXCLUDE_PATTERNS}")
add_custom_target(doc
		${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
		DEPENDS ${PROJECT_BINARY_DIR}/Doxyfile
		WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
		COMMENT "${line0}\n${line1}\n${line2}" VERBATIM
)

endif(DOXYGEN_FOUND)

