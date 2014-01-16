# Find the MySQL includes and client library
# This module defines
#  MYSQL_INCLUDE_DIR, where to find mysql.h
#  MYSQL_LIBRARIES, the libraries needed to use MySQL.
#  MYSQL_FOUND, If false, do not try to use MySQL.

set(MYSQL_CONFIG_PREFER_PATH "$ENV{MYSQL_HOME}/bin" CACHE FILEPATH
    "preferred path to MySQL (mysql_config)")
find_program(MYSQL_CONFIG mysql_config
    ${MYSQL_CONFIG_PREFER_PATH}
    /usr/local/mysql/bin/
    /usr/local/bin/
    /usr/bin/
    )

if(MYSQL_CONFIG)
    message(STATUS "Using mysql_config: ${MYSQL_CONFIG}")
    # set INCLUDE_DIR
    exec_program(${MYSQL_CONFIG}
        ARGS --include
        OUTPUT_VARIABLE MYSQL_INCLUDE_DIR)

    # set LIBRARY_DIR
    exec_program(${MYSQL_CONFIG}
        ARGS --libs_r
        OUTPUT_VARIABLE MYSQL_LIBRARIES)

else(MYSQL_CONFIG)
    # FIXME incomplete
    find_path(MYSQL_INCLUDE_DIR mysql.h
        /usr/local/include
        /usr/local/include/mysql
        /usr/local/mysql/include
        /usr/local/mysql/include/mysql
        /usr/include
        /usr/include/mysql
    #find_library(mysqlclient_r ...
    #    PATHS
    #    ${MYSQL_ADD_LIBRARY_PATH}
    #    /usr/lib/mysql
    #    /usr/local/lib
    #    /usr/local/lib/mysql
    #    /usr/local/mysql/lib
    #)
)
endif(MYSQL_CONFIG)

set(MYSQL_INCLUDE_DIR ${MYSQL_INCLUDE_DIR} CACHE FILEPATH INTERNAL)
set(MYSQL_LIBRARIES ${MYSQL_LIBRARIES} CACHE FILEPATH INTERNAL)

if(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)
    set(MYSQL_FOUND TRUE CACHE INTERNAL "MySQL found")
    message(STATUS "Found MySQL: ${MYSQL_INCLUDE_DIR}, ${MYSQL_LIBRARIES}")
else(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)
    set(MYSQL_FOUND FALSE CACHE INTERNAL "MySQL found")
    message(STATUS "MySQL not found.")
endif(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)

mark_as_advanced(MYSQL_INCLUDE_DIR MYSQL_LIBRARIES)
