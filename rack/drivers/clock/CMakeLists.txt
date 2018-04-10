option(Drivers/Clock_DCF77_EMC_Pro "Clock - Dcf77EmcPro" OFF)
option(Drivers/Clock_DCF77_MBG_C51 "Clock - Dcf77MbgC51" OFF)
option(Drivers/Clock_System "Clock - System" OFF)

if(Drivers/Clock_DCF77_EMC_Pro)
  add_executable(ClockDcf77EmcPro clock_dcf77_emc_pro.cpp)
  target_link_libraries(ClockDcf77EmcPro ${RACK_LIBS})
  install(TARGETS ClockDcf77EmcPro RUNTIME DESTINATION bin)
endif(Drivers/Clock_DCF77_EMC_Pro)

if(Drivers/Clock_DCF77_MBG_C51)
  add_executable(ClockDcf77MbgC51 clock_dcf77_mbg_c51.cpp)
  target_link_libraries(ClockDcf77MbgC51 ${RACK_LIBS})
  install(TARGETS ClockDcf77MbgC51 RUNTIME DESTINATION bin)
endif(Drivers/Clock_DCF77_MBG_C51)

if(Drivers/Clock_System)
  add_executable(ClockSystem clock_system.cpp)
  target_link_libraries(ClockSystem ${RACK_LIBS})
  install(TARGETS ClockSystem RUNTIME DESTINATION bin)
endif(Drivers/Clock_System)

