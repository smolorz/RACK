set(GUI_DRIVERS_SUBDIR "gui/drivers")

if(Build_RACK_JavaGUI)

  set(RACK_JAVA_SRC_FILES ${RACK_JAVA_SRC_FILES}
 
      ${GUI_DRIVERS_SUBDIR}/CameraComponent.java
      ${GUI_DRIVERS_SUBDIR}/CameraColorConvert.java
      ${GUI_DRIVERS_SUBDIR}/CameraMisColorConvert.java
      ${GUI_DRIVERS_SUBDIR}/CameraGui.java
      ${GUI_DRIVERS_SUBDIR}/ChassisGui.java
      ${GUI_DRIVERS_SUBDIR}/ClockGui.java
      ${GUI_DRIVERS_SUBDIR}/CompassGui.java
      ${GUI_DRIVERS_SUBDIR}/GpsGui.java
      ${GUI_DRIVERS_SUBDIR}/GyroGui.java
      ${GUI_DRIVERS_SUBDIR}/GyroComponent.java
      ${GUI_DRIVERS_SUBDIR}/IoGui.java
      ${GUI_DRIVERS_SUBDIR}/JoystickGui.java
      ${GUI_DRIVERS_SUBDIR}/JoystickSoftware.java
      ${GUI_DRIVERS_SUBDIR}/LadarGui.java
      ${GUI_DRIVERS_SUBDIR}/PtzDriveGui.java
      ${GUI_DRIVERS_SUBDIR}/ServoDriveGui.java
      ${GUI_DRIVERS_SUBDIR}/VehicleGui.java

  PARENT_SCOPE)

endif(Build_RACK_JavaGUI)

