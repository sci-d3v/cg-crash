# File utility functions for the Practicum project

function(COPY_FILES_TO_BUILD _DIRECTORY _EXTENSION)
    # Copy files into the build tree, keeping subdirectories.
    # Basename-only copy breaks nested assets: GLOB_RECURSE finds
    # assets/satellite/satellite.gltf, then configure_file looks for
    # assets/satellite.gltf and configure fails.
    message(STATUS "Copying from \"${_DIRECTORY}\" directory with \"${_EXTENSION}\" extension")
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${_DIRECTORY})

    file(GLOB_RECURSE _PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${_DIRECTORY}/*.${_EXTENSION}")
    set(_NUMBER 1)
    message(STATUS "List of Files:")
    foreach(_PATH ${_PATHS})
        file(RELATIVE_PATH _REL "${CMAKE_CURRENT_SOURCE_DIR}/${_DIRECTORY}" "${_PATH}")
        get_filename_component(_REL_DIR "${_REL}" DIRECTORY)
        if(NOT _REL_DIR STREQUAL "")
            file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${_DIRECTORY}/${_REL_DIR}")
        endif()
        configure_file("${_PATH}" "${CMAKE_CURRENT_BINARY_DIR}/${_DIRECTORY}/${_REL}" COPYONLY)
        message(STATUS "${_NUMBER}. Copying \"${_REL}\"")
        math(EXPR _NUMBER "${_NUMBER}+1")
    endforeach()
endfunction()

function(GET_FILES_BY_EXTENSION DIRECTORY EXTENSION OUTPUT_VAR)
    # Get list of files with specified extension from directory
    file(GLOB_RECURSE _FILES "${CMAKE_CURRENT_SOURCE_DIR}/${DIRECTORY}/*.${EXTENSION}")
    set(${OUTPUT_VAR} ${_FILES} PARENT_SCOPE)
endfunction()
