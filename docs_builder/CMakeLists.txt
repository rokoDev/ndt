find_package(Doxygen QUIET
             OPTIONAL_COMPONENTS dot mscgen dia)

if(NOT DOXYGEN_FOUND)
    message(STATUS "Error: Failed to find Doxygen package")
    return()
endif()

if(NOT (TARGET Doxygen::dot))
    message(STATUS "Error: Doxygen::dot does not exist")
    return()
endif()

get_target_property(PATH_TO_DOXYGEN Doxygen::doxygen IMPORTED_LOCATION)
get_target_property(PATH_TO_DOT Doxygen::dot IMPORTED_LOCATION)

# Generate project dependency graph
add_custom_command(OUTPUT "docs_images/${ProjectName}depsgraph.svg"
    COMMAND ${CMAKE_COMMAND} -E echo "Generating project dependency graph..."
    COMMAND ${CMAKE_COMMAND} -E make_directory docs_images
    COMMAND ${CMAKE_COMMAND} -E chdir docs_images ${PATH_TO_DOT} -Tsvg -o ${ProjectName}depsgraph.svg ${${ProjectName}_BINARY_DIR}/${ProjectName}deps.dot
    BYPRODUCTS "docs_images/${ProjectName}depsgraph.svg"
    VERBATIM
)

if(result)
    message(FATAL_ERROR "Failed to generate ${ProjectName} dependency graph: ${result}")
endif()

get_target_property(SRC_DIRS ${ProjectName} SRC_DIRS)

string(REPLACE ";" " " INPUT_STR "${SRC_DIRS}")
string(TOUPPER ${ProjectName} UP_ProjectName)

add_custom_target("${ProjectName}.docs.gen"
    COMMAND ${CMAKE_COMMAND} -E echo "Generating docs..."
    COMMAND ${CMAKE_COMMAND}
    -DProjectName=${UP_ProjectName}
    -DPATH_TO_DOXYGEN=${PATH_TO_DOXYGEN}
    -DPROJECT_BRIEF="${${ProjectName}_DESCRIPTION}"
    -DINPUT=${INPUT_STR}
    -DDOCS_IMAGE_PATH=${CMAKE_CURRENT_BINARY_DIR}/docs_images
    -DDOT_PATH=${PATH_TO_DOT}
    -DDOXYGEN_IN=${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
    -P ${CMAKE_CURRENT_LIST_DIR}/GenDocs.cmake
    DEPENDS "docs_images/${ProjectName}depsgraph.svg"
    BYPRODUCTS ${OUT_PATH_LIST}
    VERBATIM
)
add_dependencies("${ProjectName}.docs.gen" "${ProjectName}_updateVersion")

add_custom_target("${ProjectName}.docs.update"
    COMMAND ${CMAKE_COMMAND} -E echo "Update and commit docs to the gh-pages branch..."
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/UpdateDocs.cmake
    VERBATIM
)
add_dependencies("${ProjectName}.docs.update" "${ProjectName}.docs.gen")

add_custom_target("${ProjectName}.docs.push"
    COMMAND ${CMAKE_COMMAND} -E echo "Pushing the gh-pages branch to GitHub..."
    COMMAND ${GIT_EXECUTABLE} push origin gh-pages
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/gh-pages
    VERBATIM
)
add_dependencies("${ProjectName}.docs.push" "${ProjectName}.docs.update")

# If use IDE add ${ProjectName}_docs and ${ProjectName}.docs.push targets into documentation source group
set_target_properties(${ProjectName}.docs.gen ${ProjectName}.docs.update ${ProjectName}.docs.push PROPERTIES FOLDER documentation)