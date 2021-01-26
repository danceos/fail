if(BUILD_SAIL)
    find_package(ZLIB REQUIRED) # -lz
    if(ZLIB_FOUND)
        set(sail_library_dependencies ${sail_library_dependencies} ${ZLIB_LIBRARIES})
    endif(ZLIB_FOUND)

    #FIXME: find package GMP dynamically

    #find_package(GMP REQUIRED)
    #if(GMP_FOUND)
    #    set(sail_library_dependencies ${sail_library_dependencies} ${GMP_LIBRARIES})
    #endif(GMP_FOUND)

    set(sail_library_dependencies ${sail_library_dependencies} -lgmp)

    set(SAIL_SIMULATOR_DIR ${PROJECT_SOURCE_DIR}/simulators/sail)
    set(sail_src_dir ${SAIL_SIMULATOR_DIR}/${SAIL_ARCH})
    add_executable(fail-client  ${SAIL_SIMULATOR_DIR}/fail_empty_source_file_for_build.cc)

    # make sure aspects don't fail to match in entry.cc and the experiment headers are found
    include_directories(${PROJECT_SOURCE_DIR}/src/core ${CMAKE_BINARY_DIR}/src/core)

    set(sail_build_CC "${CMAKE_C_COMPILER}")
    set(sail_build_CFLAGS "-I${PROJECT_SOURCE_DIR}/src/core -I${CMAKE_BINARY_DIR}/src/core")
    configure_file(${PROJECT_SOURCE_DIR}/simulators/sail/sail-cc.in
               ${CMAKE_CURRENT_BINARY_DIR}/sail-cc)

    include(ExternalProject)
    ExternalProject_Add(
        libsail-emu_external
        SOURCE_DIR ${sail_src_dir}
        ## Put configure command here, to prevent cmake calling make configure
        CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo "[${PROJECT_NAME}] No configure for libsail-emu"
        BUILD_COMMAND
           ${CMAKE_COMMAND} -E env CC=${CMAKE_CURRENT_BINARY_DIR}/sail-cc $(MAKE) -C ${sail_src_dir}  ${SAIL_BUILD_ARGS}
        BUILD_IN_SOURCE 1
        BUILD_ALWAYS true
           ## Put install command here, to prevent cmake calling make install
        INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "[${PROJECT_NAME}] No install for libsail-emu"

    )

    add_library(libsail-emu STATIC IMPORTED GLOBAL)
    add_dependencies(libsail-emu libsail-emu_external)
    set_property(TARGET libsail-emu PROPERTY IMPORTED_LOCATION ${sail_src_dir}/${SAIL_SIMULATOR})

    set(SAIL_SOFTFLOAT_LIB ${SAIL_SOFTFLOAT_DIR}/softfloat.a)
    ExternalProject_Add(
        libsail-softfloat_external
        SOURCE_DIR ${sail_src_dir}
        CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo "[${PROJECT_NAME}] no configure for libsail-softfloat"
        BUILD_COMMAND
        ${CMAKE_COMMAND} -E env CC=${CMAKE_CURRENT_BINARY_DIR}/sail-cc $(MAKE) -C ${SAIL_SOFTFLOAT_DIR} ${SAIL_SOFTFLOAT_BUILD_ARGS}
        INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "[${PROJECT_NAME}] no install for libsail-softfloat"
        BUILD_IN_SOURCE 1
        )
    add_library(libsail-softfloat STATIC IMPORTED GLOBAL)
    set_property(TARGET libsail-softfloat PROPERTY IMPORTED_LOCATION ${sail_src_dir}/${SAIL_SOFTFLOAT_LIB})
    add_dependencies(libsail-softfloat libsail-softfloat_external)
	#set(sail_library_dependencies ${sail_library_dependencies}
            #libsail-softfloat)

	# add all libraries which need to be linked with libsail-emu to the target INTERFACE_LINK_LIBRARIES
	# this way they get linked too when libsail-emu gets linked to a target.
	set_property(TARGET libsail-emu PROPERTY INTERFACE_LINK_LIBRARIES ${sail_library_dependencies})

    add_dependencies(fail-client libsail-emu libsail-softfloat)
    target_link_libraries(fail-client fail libsail-emu libsail-softfloat fail-sal fail-sail fail-fsp fail-sal)
    install(TARGETS fail-client RUNTIME DESTINATION bin)

    # Get stamp directory to touch files for forcing rebuilds.
    ExternalProject_Get_Property(libsail-emu_external stamp_dir)

    add_custom_target(libsailclean
    COMMAND +make -C ${sail_src_dir} clean
    COMMAND +make -C ${sail_src_dir}/${SAIL_SOFTFLOAT_DIR} clean
    # touch stamp file to force rebuild, without calling configure again.
    COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${stamp_dir}/libsail-emu_external-configure || true
    COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${stamp_dir}/libsail-softfloat_external-configure || true
    COMMENT "[${PROJECT_NAME}] Cleaning all up (clean in sail)"
  )
endif(BUILD_SAIL)
