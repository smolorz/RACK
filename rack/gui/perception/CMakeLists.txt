set(GUI_PERCEPTION_SUBDIR "gui/perception")

if(Build_RACK_JavaGUI)

  set(RACK_JAVA_SRC_FILES ${RACK_JAVA_SRC_FILES}
 
      ${GUI_PERCEPTION_SUBDIR}/ObjRecogGui.java
      ${GUI_PERCEPTION_SUBDIR}/Scan2dGui.java
      ${GUI_PERCEPTION_SUBDIR}/Scan3dComponent.java
      ${GUI_PERCEPTION_SUBDIR}/Scan3dGui.java

  PARENT_SCOPE)

endif(Build_RACK_JavaGUI)

