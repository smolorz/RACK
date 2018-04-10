option(Drivers_Gyro_Xsens "Gyro - Xsens" OFF)

if(Drivers_Gyro_Xsens)
  add_executable(GyroXsens gyro_xsens.cpp)
  target_link_libraries(GyroXsens ${RACK_LIBS})
  install(TARGETS GyroXsens RUNTIME DESTINATION bin)
endif(Drivers_Gyro_Xsens)
