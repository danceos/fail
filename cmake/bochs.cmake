#### Add some custom targets for the autoconf-based Bochs
if(BUILD_BOCHS)

  message(STATUS "[${PROJECT_NAME}] Building Bochs variant ...")
  SET(VARIANT bochs)

  # FIXME: some of these may be mandatory, depending on the actual Bochs config!
  # -L/usr/lib -lSDL -lasound -latk-1.0 -lcairo -lfontconfig -lfreetype -lgdk_pixbuf-2.0 -lgdk-x11-2.0 -lgio-2.0 -lglib-2.0 -lgmodule-2.0 -lgobject-2.0 -lgthread-2.0 -lgtk-x11-2.0 -lICE -lm -lncurses -lpango-1.0 -lpangocairo-1.0 -lpangoft2-1.0 -lrt -lSM -lvga -lvgagl -lwx_baseu-2.8 -lwx_gtk2u_core-2.8 -lX11 -lXpm -lXrandr -pthread)
  find_package(SDL) # -lSDL
  if(SDL_FOUND)
    set(bochs_library_dependencies ${bochs_library_dependencies} ${SDL_LIBRARY})
  endif(SDL_FOUND)
  unset(FindSDL_DIR CACHE)
  unset(SDLMAIN_LIBRARY CACHE)
  unset(SDL_INCLUDE_DIR CACHE)
  unset(SDL_LIBRARY CACHE)
  find_package(ALSA) # -lasoud
  if(ALSA_FOUND)
    set(bochs_library_dependencies ${bochs_library_dependencies} ${ALSA_LIBRARIES})
  endif(ALSA_FOUND)
  unset(FindALSA_DIR CACHE)
  find_package(GTK2 COMPONENTS gtk) # -latk-1.0 -lcairo -lgdk_pixbuf-2.0 -lgdk-x11-2.0 -lglib-2.0 -lgobject-2.0 -lgtk-x11-2.0 -lpango
  if(GTK2_FOUND)
    set(bochs_library_dependencies ${bochs_library_dependencies} ${GTK2_ATK_LIBRARY} ${GTK2_CAIRO_LIBRARY} ${GTK2_GDK_PIXBUF_LIBRARY} ${GTK2_GDK_LIBRARY} ${GTK2_GLIB_LIBRARY} -lgmodule-2.0 ${GTK2_GOBJECT_LIBRARY} -lgthread-2.0 ${GTK2_GTK_LIBRARY} ${GTK2_PANGO_LIBRARY} -lpangocairo-1.0 -lpangoft2-1.0)
  endif(GTK2_FOUND)
  unset(FindGTK2_DIR CACHE)
  unset(GTK2_GIOCONFIG_INCLUDE_DIR CACHE)
  unset(GTK2_GIO_INCLUDE_DIR CACHE)
  find_package(Freetype) # -lfreetype
  if(FREETYPE_FOUND)
    set(bochs_library_dependencies ${bochs_library_dependencies} ${FREETYPE_LIBRARIES})
  endif(FREETYPE_FOUND)
  unset(FindFreetype_DIR CACHE)
  find_package(X11) # -lICE -lX11 -lXpm -lXrandr -lSM
  if(X11_FOUND)
    set(bochs_library_dependencies ${bochs_library_dependencies} ${X11_X11_LIB})
  endif(X11_FOUND)
  if(X11_ICE_FOUND)
    set(bochs_library_dependencies ${bochs_library_dependencies} ${X11_ICE_LIB})
  endif(X11_ICE_FOUND)
  if(X11_Xpm_FOUND)
    set(bochs_library_dependencies ${bochs_library_dependencies} ${X11_Xpm_LIB})
  endif(X11_Xpm_FOUND)
  if(X11_Xrandr_FOUND)
    set(bochs_library_dependencies ${bochs_library_dependencies} ${X11_Xrandr_LIB})
  endif(X11_Xrandr_FOUND)
  if(X11_SM_FOUND)
    set(bochs_library_dependencies ${bochs_library_dependencies} ${X11_SM_LIB})
  endif(X11_SM_FOUND)
  unset(FindX11_DIR CACHE)
  find_package(Curses) # -lncurses
  if(CURSES_FOUND)
    set(bochs_library_dependencies ${bochs_library_dependencies} ${CURSES_LIBRARIES})
  endif(CURSES_FOUND)
  unset(CURSES_CURSES_H_PATH CACHE)
  unset(CURSES_FORM_LIBRARY CACHE)
  unset(CURSES_HAVE_CURSES_H CACHE)
  unset(FindCurses_DIR CACHE)
  find_package(wxWidgets) # -lwx_baseu-2.8? -lwx_gtk2u_core-2.8
  if(wxWidgets_FOUND)
    set(bochs_library_dependencies ${bochs_library_dependencies} ${wxWidgets_LIBRARIES})
    link_directories(${wxWidgets_LIB_DIR})
  endif(wxWidgets_FOUND)
  unset(FindwxWidgets_DIR CACHE)
  unset(wxWidgets_USE_DEBUG CACHE)
  mark_as_advanced(wxWidgets_CONFIG_EXECUTABLE wxWidgets_wxrc_EXECUTABLE)

  find_package(VGA)
  if(VGA_FOUND)
    set(bochs_library_dependencies ${bochs_library_dependencies} ${VGA_LIBRARIES})
  endif()

  # FIXME: some libraries still need to be located the "cmake way"
  set(bochs_library_dependencies ${bochs_library_dependencies} -lfontconfig -lrt -pthread)


  set(bochs_src_dir ${PROJECT_SOURCE_DIR}/simulators/bochs)
  set(bochs_install_prefix ${bochs_src_dir}/install CACHE STRING "FailBochs installation path")
  set(bochs_configure_params --enable-a20-pin --enable-x86-64 --enable-cpu-level=6 --enable-ne2000 --enable-acpi --enable-pci --enable-usb --enable-trace-cache --enable-fast-function-calls --enable-host-specific-asms --enable-disasm --enable-readline --enable-clgd54xx --enable-fpu --enable-vmx=2 --enable-monitor-mwait --enable-cdrom --enable-sb16=linux --enable-gdb-stub --disable-docbook --with-nogui --with-x11 --with-wx --with-sdl CACHE STRING "Bochs configure parameters")

  ## Bochs CXX args for calling make
  set(bochs_build_CXX CXX=${AGXX}\ -p\ ${PROJECT_SOURCE_DIR}/src\ -p\ ${PROJECT_SOURCE_DIR}/simulators\ -p\ ${PROJECT_SOURCE_DIR}/debuggers\ -p\ ${PROJECT_SOURCE_DIR}/tools\ -p\ ${PROJECT_BINARY_DIR}/src\ -I${PROJECT_SOURCE_DIR}/src/core\ -I${CMAKE_BINARY_DIR}/src/core\ ${CMAKE_AGPP_FLAGS}\ --Xcompiler\ -std=gnu++11\ -Wno-narrowing)
  ## Bochs libtool command.
  set(bochs_build_LIBTOOL LIBTOOL=/bin/sh\ ./libtool\ --tag=CXX)

  # Use cmake's external project feature to build fail library
  include(ExternalProject)
  set_property(DIRECTORY PROPERTY EP_STEP_TARGETS configure)
  ExternalProject_Add(
    libfailbochs_external
    SOURCE_DIR ${bochs_src_dir}
    CONFIGURE_COMMAND MAKEFLAGS="" ${bochs_src_dir}/configure ${bochs_configure_params} --prefix=${bochs_install_prefix}
    PREFIX ${bochs_src_dir}
    BUILD_COMMAND $(MAKE) -C ${bochs_src_dir} ${bochs_build_CXX} ${bochs_build_LIBTOOL} libfailbochs.a
    ## Put install command here, to prevent cmake calling make install
    INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "[${PROJECT_NAME}] Built libfailbochs.a"
    BUILD_IN_SOURCE 1
  )

  # tell cmake that the external project generated a library so we can add dependencies here instead of later
  # FIXME: The following works only with cmake 2.8.4 or newer <http://public.kitware.com/Bug/view.php?id=10395>:
  #add_library(libfailbochs STATIC IMPORTED)
  #set_property(TARGET libfailbochs PROPERTY IMPORTED_LOCATION ${bochs_src_dir}/libfailbochs.a )
  #add_dependencies(libfailbochs libfailbochs_external)
  # /FIXME

  # make sure aspects don't fail to match in entry.cc
  include_directories(${PROJECT_SOURCE_DIR}/src/core ${CMAKE_BINARY_DIR}/src/core)
  # an executable needs at least one source file, so we hand over an empty .cc file to make cmake happy.
  add_executable(fail-client  ${bochs_src_dir}/fail_empty_source_file_for_build.cc)
  # FIXME: see FIXME above
  #target_link_libraries(fail-client libfailbochs fail ${bochs_library_dependencies})
  target_link_libraries(fail-client ${bochs_src_dir}/libfailbochs.a fail ${bochs_library_dependencies})
  add_dependencies(libfailbochs_external-configure fail-protoc)
  add_dependencies(libfailbochs_external libfailbochs_external-configure)
  add_dependencies(fail-client libfailbochs_external)
  add_dependencies(fail-sal    libfailbochs_external-configure)
  add_dependencies(fail-comm   libfailbochs_external-configure)
  add_dependencies(fail-util   libfailbochs_external-configure)
  # /FIXME
  install(TARGETS fail-client RUNTIME DESTINATION bin)

  # Get stamp directory to touch files for forcing rebuilds.
  ExternalProject_Get_Property(libfailbochs_external stamp_dir)

  # a few Bochs-specific passthrough targets:
  add_custom_target(bochsclean
    COMMAND +make -C ${bochs_src_dir} clean
    # touch stamp file to force rebuild, without calling configure again.
    COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${stamp_dir}/libfailbochs_external-configure || true
    COMMENT "[${PROJECT_NAME}] Cleaning all up (clean in bochs)"
  )

  add_custom_target(bochsallclean
    COMMAND +make -C ${bochs_src_dir} all-clean
    # touch stamp file to force rebuild, without calling configure again.
    COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${stamp_dir}/libfailbochs_external-configure || true
    COMMENT "[${PROJECT_NAME}] Cleaning all up (all-clean in bochs)"
  )

endif(BUILD_BOCHS)
