set(SRCS
    CapstoneDisassembler.cpp
    CapstoneDisassembler.hpp
    CapstoneToFailBochs.cpp
    CapstoneToFailBochs.hpp
    CapstoneToFailGem5.hpp
    CapstoneToFailGem5.cpp
    CapstoneToFailTranslator.cpp
    CapstoneToFailTranslator.hpp
)

include(FindCapstone)

add_library(fail-capstonedisassembler ${SRCS})

target_link_libraries(fail-capstonedisassembler ${CAPSTONE_LIBRARY})
include_directories(${CAPSTONE_INCLUDE_DIR})

### Tests
add_executable(capstoneDisTest testing/capstoneDisTest.cc)
target_link_libraries(capstoneDisTest fail-capstonedisassembler fail-sal)

add_test(NAME capstoneDisx86Test COMMAND capstoneDisTest ${CMAKE_CURRENT_SOURCE_DIR}/testing/x86 )
add_test(NAME capstoneDisx86_64Test COMMAND capstoneDisTest ${CMAKE_CURRENT_SOURCE_DIR}/testing/x86_64 )
add_test(NAME capstoneDisARMM3Test COMMAND capstoneDisTest ${CMAKE_CURRENT_SOURCE_DIR}/testing/armm3 )
add_test(NAME capstoneDisARM9Test COMMAND capstoneDisTest ${CMAKE_CURRENT_SOURCE_DIR}/testing/arm9 )