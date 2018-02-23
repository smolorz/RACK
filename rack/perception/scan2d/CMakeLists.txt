option(Perception/Scan2D_Scan2D "Scan2D" ON)
option(Perception/Scan2D_DynObjRecog "Scan2D - DynObjRecog" ON)
option(Perception/Scan2D_Merge "Scan2D - Merge" ON)
option(Perception/Scan2D_Sim "Scan2D - Sim" ON)
option(Perception/Scan2D_Lab "Scan2D - Lab" ON)

if(Perception/Scan2D_Scan2D)
  add_executable(Scan2d scan2d.cpp)
  target_link_libraries(Scan2d ${RACK_LIBS})
  install(TARGETS Scan2d RUNTIME DESTINATION bin)
endif(Perception/Scan2D_Scan2D)

if(Perception/Scan2D_DynObjRecog)
  add_executable(Scan2dDynObjRecog scan2d_dyn_obj_recog.cpp)
  target_link_libraries(Scan2dDynObjRecog ${RACK_LIBS})
  install(TARGETS Scan2dDynObjRecog RUNTIME DESTINATION bin)
endif(Perception/Scan2D_DynObjRecog)

if(Perception/Scan2D_Merge)
  add_executable(Scan2dMerge scan2d_merge.cpp)
  target_link_libraries(Scan2dMerge ${RACK_LIBS})
  install(TARGETS Scan2dMerge RUNTIME DESTINATION bin)
endif(Perception/Scan2D_Merge)

if(Perception/Scan2D_Sim)
  add_executable(Scan2dSim scan2d_sim.cpp)
  target_link_libraries(Scan2dSim ${RACK_LIBS})
  install(TARGETS Scan2dSim RUNTIME DESTINATION bin)
endif(Perception/Scan2D_Sim)

if(Perception/Scan2D_Lab)
  add_executable(Scan2dLab scan2d_lab.cpp)
  target_link_libraries(Scan2dLab ${RACK_LIBS})
  install(TARGETS Scan2dLab RUNTIME DESTINATION bin)
endif(Perception/Scan2D_Lab)


