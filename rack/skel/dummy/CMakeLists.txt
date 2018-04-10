option(Dummy_Abc "Dummy - Abc" OFF)

if(Dummy_Abc)
  add_executable(DummyAbc dummy_abc.cpp)
  target_link_libraries(DummyAbc ${RACK_LIBS})
  install(TARGETS DummyAbc RUNTIME DESTINATION bin)
endif(Dummy_Abc)

