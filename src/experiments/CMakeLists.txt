# Note that we're allowing *multiple* experiments to be enabled at once.
set(EXPERIMENTS_ACTIVATED "" CACHE STRING "Activated experiments (a semicolon-separated list of fail/src/experiments/ subdirectories)")

foreach(experiment_name ${EXPERIMENTS_ACTIVATED})
    add_subdirectory(${experiment_name})
endforeach(experiment_name)
