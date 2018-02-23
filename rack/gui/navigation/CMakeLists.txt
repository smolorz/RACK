set(GUI_NAVIGATION_SUBDIR "gui/navigation")

if(Build_RACK_JavaGUI)

  set(RACK_JAVA_SRC_FILES ${RACK_JAVA_SRC_FILES}
 
      ${GUI_NAVIGATION_SUBDIR}/FeatureMapGui.java
      ${GUI_NAVIGATION_SUBDIR}/GridMapComponent.java
      ${GUI_NAVIGATION_SUBDIR}/GridMapGui.java
      ${GUI_NAVIGATION_SUBDIR}/MclGui.java
      ${GUI_NAVIGATION_SUBDIR}/OdometryGui.java
      ${GUI_NAVIGATION_SUBDIR}/PathGui.java
      ${GUI_NAVIGATION_SUBDIR}/PilotGui.java
      ${GUI_NAVIGATION_SUBDIR}/PositionGui.java
      ${GUI_NAVIGATION_SUBDIR}/PositionNavi.java

  PARENT_SCOPE)

endif(Build_RACK_JavaGUI)

