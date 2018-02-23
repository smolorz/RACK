#add_subdirectory(camera)
add_subdirectory(chassis)
#add_subdirectory(clock)
#add_subdirectory(compass)
#add_subdirectory(gps)
#add_subdirectory(gyro)
add_subdirectory(ladar)

set(DRIVERS_SUBDIR "drivers")
set(DRIVERS_INCLUDE_DIR "${PKG_INCLUDE_DIR}/${DRIVERS_SUBDIR}")

set(DRIVERS_INCLUDE_HEADERS

    camera_proxy.h
    chassis_proxy.h
    clock_proxy.h
    compass_proxy.h
    gps_proxy.h
    gyro_proxy.h
    io_proxy.h
    joystick_proxy.h
    ladar_proxy.h
    ptz_drive_proxy.h
    servo_drive_proxy.h
    vehicle_proxy.h
)

install(FILES ${DRIVERS_INCLUDE_HEADERS} DESTINATION ${DRIVERS_INCLUDE_DIR})


if(Build_RACK_Java)

  set(RACK_JAVA_SRC_FILES ${RACK_JAVA_SRC_FILES}

      ${DRIVERS_SUBDIR}/CameraProxy.java
      ${DRIVERS_SUBDIR}/CameraDataMsg.java
      ${DRIVERS_SUBDIR}/CameraFormatMsg.java
      ${DRIVERS_SUBDIR}/ChassisProxy.java
      ${DRIVERS_SUBDIR}/ChassisParamMsg.java
      ${DRIVERS_SUBDIR}/ChassisDataMsg.java
      ${DRIVERS_SUBDIR}/ChassisSetActivePilotMsg.java
      ${DRIVERS_SUBDIR}/ClockProxy.java
      ${DRIVERS_SUBDIR}/ClockDataMsg.java
      ${DRIVERS_SUBDIR}/CompassProxy.java
      ${DRIVERS_SUBDIR}/CompassDataMsg.java
      ${DRIVERS_SUBDIR}/GpsProxy.java
      ${DRIVERS_SUBDIR}/GpsDataMsg.java
      ${DRIVERS_SUBDIR}/GyroProxy.java
      ${DRIVERS_SUBDIR}/GyroDataMsg.java
      ${DRIVERS_SUBDIR}/IoProxy.java
      ${DRIVERS_SUBDIR}/IoDataMsg.java
      ${DRIVERS_SUBDIR}/JoystickProxy.java
      ${DRIVERS_SUBDIR}/JoystickDataMsg.java
      ${DRIVERS_SUBDIR}/LadarProxy.java
      ${DRIVERS_SUBDIR}/LadarDataMsg.java
      ${DRIVERS_SUBDIR}/PtzDriveProxy.java
      ${DRIVERS_SUBDIR}/PtzDriveDataMsg.java
      ${DRIVERS_SUBDIR}/PtzDriveMovePosMsg.java
      ${DRIVERS_SUBDIR}/PtzDriveMoveVelMsg.java
      ${DRIVERS_SUBDIR}/ServoDriveProxy.java
      ${DRIVERS_SUBDIR}/ServoDriveDataMsg.java
      ${DRIVERS_SUBDIR}/ServoDriveMovePosMsg.java
      ${DRIVERS_SUBDIR}/ServoDriveMoveVelMsg.java
      ${DRIVERS_SUBDIR}/VehicleProxy.java
      ${DRIVERS_SUBDIR}/VehicleDataMsg.java
      ${DRIVERS_SUBDIR}/VehicleSetValueMsg.java

  PARENT_SCOPE)

endif(Build_RACK_Java)

        
