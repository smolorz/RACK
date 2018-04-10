set(RACK_VERSION_MAJOR 0)
set(RACK_VERSION_MINOR 6)
set(RACK_REVISION_LEVEL 90)
set(RACK_VERSION "${RACK_VERSION_MAJOR}.${RACK_VERSION_MINOR}.${RACK_REVISION_LEVEL}"
    CACHE INTERNAL "RACK version string (MAJOR.MINOR.REVISION)")
set(PACKAGE_VERSION ${RACK_VERSION} CACHE INTERNAL
    "Duplicate to RACK_VERSION, still used by old scripts and Doxygen.in.")

configure_file (
  "${PROJECT_SOURCE_DIR}/config/rack_config.h.in"
  "${PROJECT_BINARY_DIR}/config/rack_config.h"
)

