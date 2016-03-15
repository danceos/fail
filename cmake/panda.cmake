if(BUILD_PANDA)

  message(STATUS "[${PROJECT_NAME}] Building Pandaboard variant ...")
  SET(VARIANT panda)

  find_package(FTDI REQUIRED)
  set(openocd_library_dependencies ${openocd_library_dependencies} ${FTDI_LIBRARY})

  # FIXME: some libraries still need to be located the "cmake way"
  set(openocd_library_dependencies ${openocd_library_dependencies} -ldl)

  set(openocd_src_dir ${PROJECT_SOURCE_DIR}/debuggers/openocd)
  set(openocd_configure_params --enable-ft2232_libftdi --disable-werror CACHE STRING "OpenOCD default configure parameters")

  # Use cmake's external project feature to build openocd library
  include(ExternalProject)
  ExternalProject_Add(
    libfailopenocd_external
    SOURCE_DIR ${openocd_src_dir}
    CONFIGURE_COMMAND ${openocd_src_dir}/configure ${openocd_configure_params}
    BUILD_COMMAND $(MAKE) -C ${openocd_src_dir}
    ## Put install command here, to prevent cmake calling make install
    INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "[${PROJECT_NAME}] Built openocd library"
    BUILD_IN_SOURCE 1
  )

  # make sure aspects don't fail to match in entry.cc
  #include_directories(${PROJECT_SOURCE_DIR}/src/core ${CMAKE_BINARY_DIR}/src/core)
  # an executable needs at least one source file, so we hand over an empty .cc file to make cmake happy.

  set(srces ${openocd_src_dir}/openocd_wrapper.cc)

  set (srces ${srces}
    ${openocd_src_dir}/opcode_parser/arm-addressmode.c
    ${openocd_src_dir}/opcode_parser/arm-condition.c
    ${openocd_src_dir}/opcode_parser/arm-opcode-coprocessor.c
    ${openocd_src_dir}/opcode_parser/arm-opcode-data.c
    ${openocd_src_dir}/opcode_parser/arm-opcode-ldmstm-branch.c
    ${openocd_src_dir}/opcode_parser/arm-opcode-ldrstr.c
    ${openocd_src_dir}/opcode_parser/arm-opcode.c)

  add_executable(fail-client ${srces})
  target_link_libraries(fail-client ${openocd_src_dir}/src/.libs/libopenocd.a ${openocd_src_dir}/jimtcl/libjim.a fail ${openocd_library_dependencies})
  add_dependencies(fail-client libfailopenocd_external)

  # ensure, elf path is set for enabling openocd to read elf symbols
  if(EXISTS $ENV{FAIL_ELF_PATH})
    message(STATUS "[FAIL*] PandaBoard ELF under test: $ENV{FAIL_ELF_PATH}")
  else()
    message(FATAL_ERROR "Please set the FAIL_ELF_PATH environment variable to the binary under test.")
  endif()


  # copy the conf-files to build directory
  # TODO: copy to install path on install!?
  file(COPY ${openocd_src_dir}/tcl/ DESTINATION oocd_conf/ PATTERN openocd EXCLUDE)
  get_filename_component(OOCD_CONF_FILES_PATH ${CMAKE_BINARY_DIR}/oocd_conf/ REALPATH)

  configure_file(${openocd_src_dir}/openocd.cfg openocd.cfg COPYONLY)

  get_filename_component(OOCD_CONF_FILE_PATH ${CMAKE_BINARY_DIR}/openocd.cfg REALPATH)
  configure_file(${openocd_src_dir}/openocd_wrapper.hpp.in
               ${openocd_src_dir}/openocd_wrapper.hpp)
  include_directories(${openocd_src_dir})

  # /FIXME
  install(TARGETS fail-client RUNTIME DESTINATION bin)

endif(BUILD_PANDA)
