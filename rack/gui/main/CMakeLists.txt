set(GUI_MAIN_SUBDIR "gui/main")

if(Build_RACK_JavaGUI)

  set(RACK_JAVA_SRC_FILES ${RACK_JAVA_SRC_FILES}
 
      ${GUI_MAIN_SUBDIR}/GDOSGui.java
      ${GUI_MAIN_SUBDIR}/GDOSMessageRenderer.java
      ${GUI_MAIN_SUBDIR}/GDOSTable.java
      ${GUI_MAIN_SUBDIR}/MapViewActionEvent.java
      ${GUI_MAIN_SUBDIR}/MapViewComponent.java
      ${GUI_MAIN_SUBDIR}/MapViewGraphics.java
      ${GUI_MAIN_SUBDIR}/MapViewGui.java
      ${GUI_MAIN_SUBDIR}/MapViewInterface.java
      ${GUI_MAIN_SUBDIR}/NaviComponent.java
      ${GUI_MAIN_SUBDIR}/RackModuleGui.java
      ${GUI_MAIN_SUBDIR}/RackDataModuleGui.java

  PARENT_SCOPE)

endif(Build_RACK_JavaGUI)

