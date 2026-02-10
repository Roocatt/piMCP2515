find_package(Doxygen)
if (DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    add_custom_target(docgenerate
            COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} COMMENT "generating documentation" VERBATIM)

    find_package(Git QUIET)
    if(GIT_FOUND)
        execute_process(
                COMMAND git rev-parse --abbrev-ref HEAD
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE GIT_BRANCH
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if("${GIT_BRANCH}" STREQUAL "gh-pages")
            add_custom_target(docpublish
                    COMMAND ${CMAKE_COMMAND} -E rm -r ${CMAKE_CURRENT_SOURCE_DIR}/docs/
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_CURRENT_SOURCE_DIR}/doxygen/html/
                    ${CMAKE_CURRENT_SOURCE_DIR}/docs/
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} COMMENT "setting up github pages" VERBATIM)
            add_dependencies(docpublish docgenerate)
        endif()
    endif()
endif()