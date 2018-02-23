set(GUI_CONTROL_SUBDIR "gui/control")

if(Build_RACK_JavaGUI)

  set(RACK_JAVA_SRC_FILES ${RACK_JAVA_SRC_FILES}
 
      ${GUI_CONTROL_SUBDIR}/PlannerGui.java
      ${GUI_CONTROL_SUBDIR}/PlannerTable.java

  PARENT_SCOPE)

endif(Build_RACK_JavaGUI)

