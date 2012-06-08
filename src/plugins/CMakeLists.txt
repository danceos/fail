# Note that we're allowing *multiple* plugins to be enabled at once.
set(PLUGINS_ACTIVATED "" CACHE STRING "Activated plugins (a semicolon-separated list of fail/src/plugins/ subdirectories)")

foreach(plugin_name ${PLUGINS_ACTIVATED})
    add_subdirectory(${plugin_name})
endforeach(plugin_name)
