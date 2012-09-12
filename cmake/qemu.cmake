#### Add some custom targets for qemu-system-x86_64
if(BUILD_QEMU)

  message(STATUS "[${PROJECT_NAME}] Building QEMU variant ...")
  SET(VARIANT qemu)

  # ./configure --prefix=$(echo ~/localroot/usr) --enable-sdl --disable-vnc --disable-curses --disable-curl --enable-system --target-list=x86_64-softmmu
  # LIBS = -lrt -pthread -lgthread-2.0 -lglib-2.0 -lutil -luuid -lSDL -lX11 -lm -lz

  # -L/usr/lib -lSDL -lasound -latk-1.0 -lcairo -lfontconfig -lfreetype -lgdk_pixbuf-2.0 -lgdk-x11-2.0 -lgio-2.0 -lglib-2.0 -lgmodule-2.0 -lgobject-2.0 -lgthread-2.0 -lgtk-x11-2.0 -lICE -lm -lncurses -lpango-1.0 -lpangocairo-1.0 -lpangoft2-1.0 -lrt -lSM -lvga -lvgagl -lwx_baseu-2.8 -lwx_gtk2u_core-2.8 -lX11 -lXpm -lXrandr -pthread)
  find_package(SDL) # -lSDL
  if(SDL_FOUND)
    set(qemu_library_dependencies ${qemu_library_dependencies} ${SDL_LIBRARY})
  endif(SDL_FOUND)
  unset(FindSDL_DIR CACHE)
  unset(SDLMAIN_LIBRARY CACHE)
  unset(SDL_INCLUDE_DIR CACHE)
  unset(SDL_LIBRARY CACHE)
  find_package(GTK2 COMPONENTS gtk)
  if(GTK2_FOUND)
    set(qemu_library_dependencies ${qemu_library_dependencies} ${GTK2_GLIB_LIBRARY} -lgthread-2.0)
  endif(GTK2_FOUND)
  unset(FindGTK2_DIR CACHE)
  unset(GTK2_GIOCONFIG_INCLUDE_DIR CACHE)
  unset(GTK2_GIO_INCLUDE_DIR CACHE)
  find_package(X11) # -lX11
  if(X11_FOUND)
    set(qemu_library_dependencies ${qemu_library_dependencies} ${X11_X11_LIB})
  endif(X11_FOUND)
  find_package(ZLIB) # -lz
  if(ZLIB_FOUND)
    set(qemu_library_dependencies ${qemu_library_dependencies} ${ZLIB_LIBRARIES})
  endif(ZLIB_FOUND)
  find_package(LibUUID) # -luuid
  if(UUID_FOUND)
    set(qemu_library_dependencies ${qemu_library_dependencies} ${UUID_LIBRARIES})
  endif(UUID_FOUND)

  # FIXME: some libraries still need to be located the "cmake way"
  set(qemu_library_dependencies ${qemu_library_dependencies} -lrt -pthread -lutil -lm)

  set(qemu_src_dir ${PROJECT_SOURCE_DIR}/simulators/qemu)
  set(qemu_lib "${qemu_src_dir}/x86_64-softmmu/qemu-system-x86_64.a")

  add_custom_command(OUTPUT "${qemu_lib}"
    COMMAND +make -C ${qemu_src_dir} CFLAGS=\"-I${PROJECT_SOURCE_DIR}/src/core -I${CMAKE_BINARY_DIR}/src/core\"
    COMMENT "[${PROJECT_NAME}] Building qemu-system-x86_64.a"
  )

  # make sure aspects don't fail to match in entry.cc
  include_directories(${PROJECT_SOURCE_DIR}/src/core ${CMAKE_BINARY_DIR}/src/core)
  add_executable(fail-client "${qemu_lib}")
  target_link_libraries(fail-client -Wl,-whole-archive "${qemu_lib}" -Wl,-no-whole-archive fail ${qemu_library_dependencies})
  install(TARGETS fail-client RUNTIME DESTINATION bin)
  
  # a few QEMU-specific passthrough targets:
  add_custom_target(qemuclean
    COMMAND +make -C ${qemu_src_dir} clean
    COMMENT "[${PROJECT_NAME}] Cleaning all up (clean in qemu)"
  )

endif(BUILD_QEMU)
