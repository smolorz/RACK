add_subdirectory(datalog)

set(TOOLS_SUBDIR "tools")
set(TOOLS_INCLUDE_DIR "${PKG_INCLUDE_DIR}/${TOOLS_SUBDIR}")

set(TOOLS_INCLUDE_HEADERS

    datalog_proxy.h
)

install(FILES ${TOOLS_INCLUDE_HEADERS} DESTINATION ${TOOLS_INCLUDE_DIR})


if(Build_RACK_Java)

  set(RACK_JAVA_SRC_FILES ${RACK_JAVA_SRC_FILES}
 
      ${TOOLS_SUBDIR}/Camera2PngConverter.java
      ${TOOLS_SUBDIR}/DatalogLogInfo.java
      ${TOOLS_SUBDIR}/DatalogDataMsg.java
      ${TOOLS_SUBDIR}/DatalogProxy.java

  PARENT_SCOPE)

endif(Build_RACK_Java)

