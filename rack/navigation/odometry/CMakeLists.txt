option(Navigation_Odometry_Chassis "Odometry - Chassis" ON)

if(Navigation_Odometry_Chassis)
  add_executable(OdometryChassis odometry_chassis.cpp)
  target_link_libraries(OdometryChassis ${RACK_LIBS})
  install(TARGETS OdometryChassis RUNTIME DESTINATION bin)
endif(Navigation_Odometry_Chassis)

