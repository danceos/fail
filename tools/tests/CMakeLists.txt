
option(ENABLE_DATABASE_TESTS "Perform tests that require a MySQL Database?" OFF)

# CREATE DATABASE fail_test;
# GRANT ALL ON fail_test.* TO 'fail_test'@'localhost' IDENTIFIED BY 'fail_test' WITH GRANT OPTION;
set(TEST_MYSQL_HOST   "localhost" CACHE STRING "")
set(TEST_MYSQL_USER     "fail_test" CACHE STRING "")
set(TEST_MYSQL_PASSWORD "fail_test" CACHE STRING "")
set(TEST_MYSQL_DATABASE "fail_test" CACHE STRING "")
set(TEST_MYSQL_PORT     "3306" CACHE STRING "")

if(ENABLE_DATABASE_TESTS)

configure_file("my.cnf.in" "my.cnf")
set(TEST_DRIVER_ARGS ${TEST_DRIVER_ARGS} --my-cnf ${CMAKE_CURRENT_BINARY_DIR}/my.cnf)

foreach(BENCHMARK fib qsort)
  add_test(
    NAME    dump-trace-${BENCHMARK}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ./run ${TEST_DRIVER_ARGS} dump-trace ${BENCHMARK}
    )
  set_tests_properties(dump-trace-${BENCHMARK} PROPERTIES SKIP_RETURN_CODE 127)


  add_test(
    NAME    basic-pruner-${BENCHMARK}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ./run ${TEST_DRIVER_ARGS} basic-pruner ${BENCHMARK}
    )
  set_tests_properties(basic-pruner-${BENCHMARK} PROPERTIES SKIP_RETURN_CODE 127)

endforeach()

endif()
