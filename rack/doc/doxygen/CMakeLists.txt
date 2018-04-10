configure_file (
  "${PROJECT_SOURCE_DIR}/doc/doxygen/Doxyfile.in"
  "${PROJECT_BINARY_DIR}/doc/doxygen/Doxyfile"
)

add_custom_target(doc ALL ${DOXYGEN_EXECUTABLE})

install(FILES ${PROJECT_BINARY_DIR}/doc/doxygen/rack_doc.tag
        DESTINATION share/doc/${PROJECT_NAME})
install(DIRECTORY ${PROJECT_BINARY_DIR}/doc/doxygen/html
        DESTINATION share/doc/${PROJECT_NAME} USE_SOURCE_PERMISSIONS)
