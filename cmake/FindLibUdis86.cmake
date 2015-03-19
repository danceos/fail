# - Try to find udis86 library (udis86.sourceforge.net)
# Once done this will define
#
#  LIBUDIS86_FOUND - system has libudis86
#  LIBUDIS86_INCLUDE_DIRS - the libudis86 include directory
#  LIBUDIS86_LIBRARIES - Link these to use libudis86
#  LIBUDIS86_DEFINITIONS - Compiler switches required for using libudis86

FIND_PATH(LIBUDIS86_INCLUDE_DIRS udis86.h PATHS ${LIBUDIS86_PREFIX_DIR}/include)

FIND_LIBRARY(LIBUDIS86_LIBRARIES NAMES udis86
	PATHS /usr/lib /usr/local/lib /opt/local/lib ${LIBUDIS86_PREFIX_DIR}/lib
	ENV LIBRARY_PATH   # PATH and LIB will also work
	ENV LD_LIBRARY_PATH)

# handle the QUIETLY and REQUIRED arguments and set LIBUDIS86_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(libudis86 DEFAULT_MSG LIBUDIS86_LIBRARIES LIBUDIS86_INCLUDE_DIRS)

MARK_AS_ADVANCED(LIBUDIS86_INCLUDE_DIR LIBUDIS86_INCLUDE_DIRS LIBUDIS86_LIBRARIES)
