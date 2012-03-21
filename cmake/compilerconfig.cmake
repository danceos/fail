
##### Verbose make ####
option( VERBOSE_MAKE "Verbose Makefile output" OFF) # defaults to OFF
	set(CMAKE_VERBOSE_MAKEFILE ${VERBOSE_MAKE})


##### Compilers.. #####
SET( COMPILER "ag++" CACHE STRING "Use clang/gcc/ag++") # Defaults to ag++
#SET( PARALLELBUILDS "4" CACHE STRING "Parallel builds forc compiling Bochs (-j N)") 

if(${COMPILER} STREQUAL "clang")
  set(CMAKE_C_COMPILER "clang")
  set(CMAKE_CXX_COMPILER "clang++")

elseif(${COMPILER} STREQUAL "gcc")
  set(CMAKE_C_COMPILER "gcc")
  set(CMAKE_CXX_COMPILER "g++")

elseif(${COMPILER} STREQUAL "ag++")
  set(CMAKE_C_COMPILER "ag++")
  set(CMAKE_CXX_COMPILER "ag++")
  ## Here we add the build dir holding the generated header files (protobuf)
  add_definitions("-p ${CMAKE_SOURCE_DIR} --real-instances --Xcompiler" )

else(${COMPILER} STREQUAL "clang")
  message(FATAL_ERROR "COMPILER must be exactly one of clang/gcc/ag++.  If unsure, use 'ag++'.")
endif(${COMPILER} STREQUAL "clang")

message(STATUS "[${PROJECT_NAME}] Compiler: ${CMAKE_C_COMPILER}/${CMAKE_CXX_COMPILER}" )

#### Add some custom targets for the autoconf-based Bochs
if(BUILD_BOCHS)

  set(bochs_src_dir ${CMAKE_CURRENT_SOURCE_DIR}/bochs )

  add_custom_target( bochsclean
  		COMMAND +make -C ${bochs_src_dir} clean
   		COMMENT "[${PROJECT_NAME}] Cleaning all up (clean in bochs)"
  )
  
  add_custom_target( bochsallclean
  		COMMAND +make -C ${bochs_src_dir} all-clean
  		COMMENT "[${PROJECT_NAME}] Cleaning all up (all-clean in bochs)"
  )
  
  add_custom_target( bochs
  		COMMAND +make -C ${bochs_src_dir} CXX=\"ag++ -p ${CMAKE_SOURCE_DIR} -I${CMAKE_SOURCE_DIR}/core -I${CMAKE_BINARY_DIR}/core --real-instances --Xcompiler\" LIBTOOL=\"/bin/sh ./libtool --tag=CXX\"
  		COMMENT "[${PROJECT_NAME}] Building Bochs"
  )
  add_dependencies(bochs fail)
  
  
  add_custom_target( bochsinstall
  		COMMAND +make -C ${bochs_src_dir} CXX=\"ag++ -p ${CMAKE_SOURCE_DIR} -I${CMAKE_SOURCE_DIR}/core -I${CMAKE_BINARY_DIR}/core --real-instances --Xcompiler\" LIBTOOL=\"/bin/sh ./libtool --tag=CXX\" install
  		COMMENT "[${PROJECT_NAME}] Installing Bochs..."
  )

  add_custom_target( bochsuninstall
  		COMMAND +make -C ${bochs_src_dir} uninstall
  		COMMENT "[${PROJECT_NAME}] Uninstalling Bochs..."
  )

endif(BUILD_BOCHS)

