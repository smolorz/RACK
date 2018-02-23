set(GUI_TOOLS_SUBDIR "gui/tools")

if(Build_RACK_JavaGUI)

  set(RACK_JAVA_SRC_FILES ${RACK_JAVA_SRC_FILES}
 
      ${GUI_TOOLS_SUBDIR}/DatalogGui.java

  PARENT_SCOPE)

endif(Build_RACK_JavaGUI)

