# - Try to find PCL library (Portable Coroutine Library, libpcl1)
# Once done this will define
#
#  LIBPCL_FOUND - system has libPCL
#  LIBPCL_INCLUDE_DIRS - the libPCL include directory
#  LIBPCL_LIBRARIES - Link these to use libPCL
#  LIBPCL_DEFINITIONS - Compiler switches required for using libPCL

FIND_PATH(LIBPCL_INCLUDE_DIRS pcl.h)

FIND_LIBRARY(LIBPCL_LIBRARIES NAMES pcl
	PATHS /usr/lib /usr/local/lib /opt/local/lib
	ENV LIBRARY_PATH   # PATH and LIB will also work
	ENV LD_LIBRARY_PATH)

# handle the QUIETLY and REQUIRED arguments and set LIBPCL_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(libPCL DEFAULT_MSG LIBPCL_LIBRARIES LIBPCL_INCLUDE_DIRS)

MARK_AS_ADVANCED(LIBPCL_INCLUDE_DIRS LIBPCL_LIBRARIES)
unset(libPCL_DIR CACHE)
