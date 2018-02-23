option(Navigation_Pilot_Joystick "Pilot - Joystick" ON)
option(Navigation_Pilot_WallFollowing "Pilot - WallFollowing" ON)
option(Navigation_Pilot_Lab "Pilot - Lab" ON)

if(Navigation_Pilot_Joystick)
  add_executable(PilotJoystick pilot_joystick.cpp)
  target_link_libraries(PilotJoystick ${RACK_LIBS})
  install(TARGETS PilotJoystick RUNTIME DESTINATION bin)
endif(Navigation_Pilot_Joystick)

if(Navigation_Pilot_WallFollowing)
  add_executable(PilotWallFollowing pilot_wall_following.cpp)
  target_link_libraries(PilotWallFollowing ${RACK_LIBS})
  install(TARGETS PilotWallFollowing RUNTIME DESTINATION bin)
endif(Navigation_Pilot_WallFollowing)

if(Navigation_Pilot_Lab)
  add_executable(PilotLab pilot_lab.cpp)
  target_link_libraries(PilotLab ${RACK_LIBS})
  install(TARGETS PilotLab RUNTIME DESTINATION bin)
endif(Navigation_Pilot_Lab)

