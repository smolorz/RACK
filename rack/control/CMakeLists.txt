set(CONTROL_SUBDIR "control")
set(CONTROL_INCLUDE_DIR "${PKG_INCLUDE_DIR}/${CONTROL_SUBDIR}")

set(CONTROL_INCLUDE_HEADERS

    planner_proxy.h
)

install(FILES ${CONTROL_INCLUDE_HEADERS} DESTINATION ${CONTROL_INCLUDE_DIR})


if(Build_RACK_Java)

  set(RACK_JAVA_SRC_FILES ${RACK_JAVA_SRC_FILES}
 
      ${CONTROL_SUBDIR}/PlannerCommandMsg.java
      ${CONTROL_SUBDIR}/PlannerDataMsg.java
      ${CONTROL_SUBDIR}/PlannerProxy.java
      ${CONTROL_SUBDIR}/PlannerString.java

  PARENT_SCOPE)

endif(Build_RACK_Java)

