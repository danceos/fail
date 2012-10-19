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

  # FIXME: some libraries still need to be located the "cmake way"
  set(bochs_library_dependencies ${bochs_library_dependencies} -lfontconfig -lrt -lvga -lvgagl -pthread)

  set(bochs_src_dir ${PROJECT_SOURCE_DIR}/simulators/bochs)

  add_custom_command(OUTPUT "${bochs_src_dir}/libfailbochs.a"
    COMMAND +make -C ${bochs_src_dir} CXX=\"ag++ -p ${PROJECT_SOURCE_DIR} -I${PROJECT_SOURCE_DIR}/src/core -I${CMAKE_BINARY_DIR}/src/core ${CMAKE_AGPP_FLAGS} --Xcompiler\" LIBTOOL=\"/bin/sh ./libtool --tag=CXX\" libfailbochs.a
    COMMENT "[${PROJECT_NAME}] Building libfailbochs"
  )

  # make sure aspects don't fail to match in entry.cc
  include_directories(${PROJECT_SOURCE_DIR}/src/core ${CMAKE_BINARY_DIR}/src/core)
  add_executable(fail-client "${bochs_src_dir}/libfailbochs.a")
  target_link_libraries(fail-client "${bochs_src_dir}/libfailbochs.a" fail ${bochs_library_dependencies})
  install(TARGETS fail-client RUNTIME DESTINATION bin)
  
  # a few Bochs-specific passthrough targets:
  add_custom_target(bochsclean
    COMMAND +make -C ${bochs_src_dir} clean
    COMMENT "[${PROJECT_NAME}] Cleaning all up (clean in bochs)"
  )
  
  add_custom_target(bochsallclean
    COMMAND +make -C ${bochs_src_dir} all-clean
    COMMENT "[${PROJECT_NAME}] Cleaning all up (all-clean in bochs)"
  )
  
  # these don't work, because we don't build a bochs binary anymore:
  #add_custom_target(bochsinstall
  #  COMMAND +make -C ${bochs_src_dir} CXX=\"ag++ -p ${PROJECT_SOURCE_DIR} -I${PROJECT_SOURCE_DIR}/src/core -I${CMAKE_BINARY_DIR}/src/core --real-instances --Xcompiler\" LIBTOOL=\"/bin/sh ./libtool --tag=CXX\" install
  #  COMMENT "[${PROJECT_NAME}] Installing Bochs ..."
  #)
  #
  #add_custom_target(bochsuninstall
  #  COMMAND +make -C ${bochs_src_dir} uninstall
  #  COMMENT "[${PROJECT_NAME}] Uninstalling Bochs ..."
  #)

endif(BUILD_BOCHS)
