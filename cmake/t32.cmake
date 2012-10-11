#### Add some custom targets for T32
if(BUILD_T32)

  message(STATUS "[${PROJECT_NAME}] Building T32 variant ...")
  SET(VARIANT t32)

  # make sure aspects don't fail to match in entry.cc
  include_directories(${PROJECT_SOURCE_DIR}/src/core ${CMAKE_BINARY_DIR}/src/core)

endif(BUILD_T32)
