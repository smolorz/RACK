add_subdirectory(main)
add_subdirectory(control)
add_subdirectory(drivers)
add_subdirectory(navigation)
add_subdirectory(perception)
add_subdirectory(tools)

set(GUI_SUBDIR "gui")

if(Build_RACK_JavaGUI)

  set(RACK_JAVA_SRC_FILES ${RACK_JAVA_SRC_FILES}
 
      ${GUI_SUBDIR}/AppletGui.java
      ${GUI_SUBDIR}/Gui.java
      ${GUI_SUBDIR}/GuiCfg.java
      ${GUI_SUBDIR}/GuiElement.java
      ${GUI_SUBDIR}/GuiElementDescriptor.java
      ${GUI_SUBDIR}/GuiGroupDescriptor.java
      ${GUI_SUBDIR}/GuiWorkspaceDescriptor.java
 
  PARENT_SCOPE)

endif(Build_RACK_JavaGUI)

