set(LIBRACK_SOURCES

    version_string.cpp

    ${PROJECT_SOURCE_DIR}/control/planner_proxy.cpp

    ${PROJECT_SOURCE_DIR}/drivers/camera_proxy.cpp
    ${PROJECT_SOURCE_DIR}/drivers/chassis_proxy.cpp
    ${PROJECT_SOURCE_DIR}/drivers/clock_proxy.cpp
    ${PROJECT_SOURCE_DIR}/drivers/compass_proxy.cpp
    ${PROJECT_SOURCE_DIR}/drivers/gps_proxy.cpp
    ${PROJECT_SOURCE_DIR}/drivers/gyro_proxy.cpp
    ${PROJECT_SOURCE_DIR}/drivers/io_proxy.cpp
    ${PROJECT_SOURCE_DIR}/drivers/joystick_proxy.cpp
    ${PROJECT_SOURCE_DIR}/drivers/ladar_proxy.cpp
    ${PROJECT_SOURCE_DIR}/drivers/ptz_drive_proxy.cpp
    ${PROJECT_SOURCE_DIR}/drivers/servo_drive_proxy.cpp
    ${PROJECT_SOURCE_DIR}/drivers/vehicle_proxy.cpp
 
    ${PROJECT_SOURCE_DIR}/perception/obj_recog_proxy.cpp
    ${PROJECT_SOURCE_DIR}/perception/scan2d_proxy.cpp
    ${PROJECT_SOURCE_DIR}/perception/scan3d_proxy.cpp

    ${PROJECT_SOURCE_DIR}/navigation/feature_map_proxy.cpp
    ${PROJECT_SOURCE_DIR}/navigation/grid_map_proxy.cpp
    ${PROJECT_SOURCE_DIR}/navigation/mcl_proxy.cpp
    ${PROJECT_SOURCE_DIR}/navigation/odometry_proxy.cpp
    ${PROJECT_SOURCE_DIR}/navigation/path_proxy.cpp
    ${PROJECT_SOURCE_DIR}/navigation/pilot_proxy.cpp
    ${PROJECT_SOURCE_DIR}/navigation/position_proxy.cpp

    ${PROJECT_SOURCE_DIR}/tools/datalog_proxy.cpp
    ${PROJECT_SOURCE_DIR}/tools/datalog/datalog_rec_class.cpp

    ${PROJECT_SOURCE_DIR}/main/tools/argopts.cpp
    ${PROJECT_SOURCE_DIR}/main/tools/dxf_map.cpp
    ${PROJECT_SOURCE_DIR}/main/tools/position_tool.cpp
    ${PROJECT_SOURCE_DIR}/main/tools/camera_tool.cpp
    ${PROJECT_SOURCE_DIR}/main/tools/compress_tool.cpp
    ${PROJECT_SOURCE_DIR}/main/tools/scan3d_compress_tool.cpp

    ${PROJECT_SOURCE_DIR}/main/common/rack_mailbox.cpp
    ${PROJECT_SOURCE_DIR}/main/common/rack_module.cpp
    ${PROJECT_SOURCE_DIR}/main/common/rack_data_module.cpp
    ${PROJECT_SOURCE_DIR}/main/common/rack_proxy.cpp
)

if(Build_OS_Xenomai)

  set(LIBRACK_SOURCES ${LIBRACK_SOURCES}

      ${PROJECT_SOURCE_DIR}/main/xenomai/rack_mutex_xenomai.cpp
      ${PROJECT_SOURCE_DIR}/main/xenomai/rack_task_xenomai.cpp
      ${PROJECT_SOURCE_DIR}/main/xenomai/rack_time_xenomai.cpp
      ${PROJECT_SOURCE_DIR}/main/xenomai/serial_port_xenomai.cpp
      ${PROJECT_SOURCE_DIR}/main/xenomai/can_port_xenomai.cpp
      ${PROJECT_SOURCE_DIR}/main/tims/xenomai/tims_api_xenomai.c
  )

  set(RACK_LINK_LIBS ${XENOMAI_LIBS})

else()

  set(LIBRACK_SOURCES ${LIBRACK_SOURCES}

      ${PROJECT_SOURCE_DIR}/main/linux/rack_mutex_linux.cpp
      ${PROJECT_SOURCE_DIR}/main/linux/rack_task_linux.cpp
      ${PROJECT_SOURCE_DIR}/main/linux/rack_time_linux.cpp
      ${PROJECT_SOURCE_DIR}/main/linux/serial_port_linux.cpp
      ${PROJECT_SOURCE_DIR}/main/linux/can_port_linux.cpp
      ${PROJECT_SOURCE_DIR}/main/tims/linux/tims_api_linux.c
  )

  set(RACK_LINK_LIBS ${LINUX_LIBS})

endif(Build_OS_Xenomai)

add_library(${PROJECT_NAME} SHARED ${LIBRACK_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${RACK_VERSION})
target_compile_definitions(${PROJECT_NAME} PUBLIC ${RACK_DEFS})
target_link_libraries(${PROJECT_NAME} ${RACK_LINK_LIBS})
install(TARGETS ${PROJECT_NAME} DESTINATION lib)

set(RACK_LIBS ${PROJECT_NAME} CACHE INTERNAL "the set of built RACK libraries")

