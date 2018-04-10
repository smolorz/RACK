option(Perception/ObjRecog_Ibeo_Lux "Object Recognition - Ibeo Lux" OFF)
option(Perception/ObjRecog_Relay_Ladar "Object Recognition - RelayLadar" OFF)

if(Perception/ObjRecog_Ibeo_Lux)
  add_executable(ObjRecogIbeoLux obj_recog_ibeo_lux.cpp)
  target_link_libraries(ObjRecogIbeoLux ${RACK_LIBS})
  install(TARGETS ObjRecogIbeoLux RUNTIME DESTINATION bin)
endif(Perception/ObjRecog_Ibeo_Lux)

if(Perception/ObjRecog_Relay_Ladar)
  add_executable(ObjRecogRelayLadar obj_recog_relay_ladar.cpp)
  target_link_libraries(ObjRecogRelayLadar ${RACK_LIBS})
  install(TARGETS ObjRecogRelayLadar RUNTIME DESTINATION bin)
endif(Perception/ObjRecog_Relay_Ladar)

