# - Try to find UUID (Universally Unique Identifier)
# Once done this will define
#
#  UUID_FOUND - system has UUID
#  UUID_INCLUDE_DIRS - the UUID include directory
#  UUID_LIBRARIES - Link these to use UUID
#  UUID_DEFINITIONS - Compiler switches required for using UUID
#
#  Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

if (UUID_LIBRARIES AND UUID_INCLUDE_DIRS)
  # in cache already
  set(UUID_FOUND TRUE)
else (UUID_LIBRARIES AND UUID_INCLUDE_DIRS)
  find_path(UUID_INCLUDE_DIR
    NAMES
      uuid/uuid.h
    PATHS
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
  )

  find_library(UUID_LIBRARY
    NAMES
      uuid
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  set(UUID_INCLUDE_DIRS
    ${UUID_INCLUDE_DIR}
  )
  set(UUID_LIBRARIES
    ${UUID_LIBRARY}
  )

  if (UUID_INCLUDE_DIRS AND UUID_LIBRARIES)
    set(UUID_FOUND TRUE)
  endif (UUID_INCLUDE_DIRS AND UUID_LIBRARIES)

  if (UUID_FOUND)
    if (NOT UUID_FIND_QUIETLY)
      message(STATUS "Found UUID (Universally Unique Identifier): ${UUID_LIBRARIES}")
    endif (NOT UUID_FIND_QUIETLY)
  else (UUID_FOUND)
    if (UUID_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find UUID (Universally Unique Identifier)")
    endif (UUID_FIND_REQUIRED)
  endif (UUID_FOUND)

  # show the UUID_INCLUDE_DIRS and UUID_LIBRARIES variables only in the advanced view
  mark_as_advanced(UUID_INCLUDE_DIRS UUID_LIBRARIES)

endif (UUID_LIBRARIES AND UUID_INCLUDE_DIRS)
