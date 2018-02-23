option(Navigation_Position "Position" ON)

if(Navigation_Position)
  add_executable(Position position.cpp)
  target_link_libraries(Position ${RACK_LIBS})
  install(TARGETS Position RUNTIME DESTINATION bin)
endif(Navigation_Position)

