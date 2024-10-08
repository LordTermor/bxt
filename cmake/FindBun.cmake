################################################################################
# Bun Configuration: Find Bun executable and set up related functions
################################################################################

set(_prev "${BUN_EXECUTABLE}")
find_program(BUN_EXECUTABLE bun DOC "Path to Bun, the all-in-one JavaScript runtime")

if(BUN_EXECUTABLE)
    if(NOT _prev)
        message(STATUS "Found Bun executable: ${BUN_EXECUTABLE}")
    endif()
    set(Bun_FOUND TRUE CACHE INTERNAL "")
else()
    set(Bun_FOUND FALSE CACHE INTERNAL "")
    if(Bun_FIND_REQUIRED)
        message(FATAL_ERROR "Failed to find a Bun executable")
    endif()
endif()

################################################################################
# Bun Utility Functions: Define functions for running Bun commands
################################################################################

function(bun_run_command cmd working_dir)
  message(STATUS "Running command: ${BUN_EXECUTABLE} ${cmd} at ${working_dir}")
  execute_process(
    COMMAND ${BUN_EXECUTABLE} ${cmd}
    RESULT_VARIABLE result
    ERROR_VARIABLE error
    WORKING_DIRECTORY ${working_dir}
  )
  if (result)
    message(FATAL_ERROR "Failed to run command: ${result}")
  endif()
endfunction()

function(bun_add_vite_project package_name working_dir)
  bun_run_command(install ${working_dir})

  add_custom_target("${package_name}_build" ALL
    COMMAND ${CMAKE_COMMAND} -E env BUILD_PATH='${CMAKE_CURRENT_BINARY_DIR}' ${BUN_EXECUTABLE} run --bun vite-build
    WORKING_DIRECTORY ${working_dir}
    COMMENT "Building ${package_name}"
    USES_TERMINAL
  )

  add_custom_command(
        TARGET "${package_name}_build" POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_BINARY_DIR}
                ${CMAKE_BINARY_DIR}/bin/web)

  add_custom_target("${package_name}_install"
    COMMAND ${CMAKE_COMMAND} -E env BUILD_PATH='${CMAKE_CURRENT_BINARY_DIR}' ${BUN_EXECUTABLE} install
    WORKING_DIRECTORY ${working_dir}
    COMMENT "Installing packages for ${package_name}"
    USES_TERMINAL
  )
endfunction()
