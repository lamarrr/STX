
#if(STX_BUILD_DOCS)
#find_package(Doxygen) # sub-packages?
#if(Doxygen_FOUND)

#set(STX_DOXYGEN_IN ${CMAKE_DIR}/Doxyfile.in)
#set(STX_DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/docs/Doxyfile)

# request to configure the file
#configure_file(${STX_DOXYGEN_IN} ${STX_DOXYGEN_OUT} @ONLY)
#message(STATUS "Doxygen build started")

# note the option ALL which allows to build the docs together with the
# application
#add_custom_target(
#  docs ALL
#  COMMAND ${DOXYGEN_EXECUTABLE} ${STX_DOXYGEN_OUT}
#  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
#  COMMENT "Generating API documentation with Doxygen"
#  VERBATIM)

#file(COPY assets/fonts assets/images
#     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/docs/html/assets)


#else()
#  message(
#    WARNING "Doxygen can not be found, Documentation will not be built.")
#  set(STX_BUILD_DOCS
#      OFF
#      CACHE BOOL "Build documentation" FORCE)
#endif()
#endif()



