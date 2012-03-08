#### Configuration file emitting BUILD_OVP/BOCHS defines ####
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/variant_config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/variant_config.h)
