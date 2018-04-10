option(Drivers_Compass_Cmps03 "Compass - Cmps03" OFF)

if(Drivers_Compass_Cmps03)
  add_executable(CompassCmps03 compass_cmps03.cpp)
  target_link_libraries(CompassCmps03 ${RACK_LIBS})
  install(TARGETS CompassCmps03 RUNTIME DESTINATION bin)
endif(Drivers_Compass_Cmps03)
