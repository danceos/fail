#### gem5-specific stuff
if(BUILD_GEM5)
  message(STATUS "[${PROJECT_NAME}] Building gem5 variant ...")
  SET(VARIANT gem5)

  #set(gem5_src_dir ${PROJECT_SOURCE_DIR}/simulators/gem5)
endif(BUILD_GEM5)
