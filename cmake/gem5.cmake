#### gem5-specific stuff
if(BUILD_GEM5)
  message(STATUS "[${PROJECT_NAME}] Building gem5 variant ...")
  SET(VARIANT gem5)

  set(gem5_src_dir       ${PROJECT_SOURCE_DIR}/simulators/gem5)
  set(gem5_wrapper       ${PROJECT_SOURCE_DIR}/src/core/sal/gem5)
  set(gem5_build_config  build/ARM/gem5.debug)
  set(core_count         9)

  # FIXMEs:
  # - incremental builds working?
  # - dependency for modified .cc files not correctly checked
  # - core_count should be derived from the parent make -jX parameter
  # - make gem5_build_config configurable in CMake
  #   (alternative: gem5_build_config is set based on the CMake build
  #    config, e.g., "Debug" or "Release")
  # - Ideally, there is no additional "gem5-clean" target. Instead,
  #   calling "make clean" should also invoke a clean command in the
  #   gem5 root dir. This seems easy for "make only" projects--things
  #   get shaky due to "scons".

  # Enable ExternalProject CMake module
  include(ExternalProject)

  # Use cmake's external project feature to build gem5 (and link FailGem5)
  ExternalProject_Add(
    FailGem5_binary_external # the (unique) name of this custom target (= external project)
    # Disable update, patch and configure step:
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CONFIGURE_COMMAND ""
    SOURCE_DIR ${gem5_src_dir}
    BINARY_DIR ${gem5_src_dir}
    # Build gem5 using scons build system:
    BUILD_COMMAND scons "CXX=${CMAKE_CXX_COMPILER} -p ${gem5_src_dir} --Xcompiler" EXTRAS=${gem5_wrapper} ${gem5_build_config} -j${core_count}
    # Disable install step (for now)
    INSTALL_COMMAND ""
  )
  add_custom_target(gem5-allclean
    COMMAND @echo "Cleaning Fail* and gem5 ..."
    COMMAND cd "${PROJECT_BINARY_DIR}/" && make clean
    COMMAND cd "${gem5_src_dir}/" && scons -c
  )

  # Build "fail" library first (will be statically linked to gem5)
  add_dependencies(FailGem5_binary_external fail)
endif(BUILD_GEM5)
