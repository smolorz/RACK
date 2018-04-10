option(Drivers_GPS_NMEA "GPS - NMEA" OFF)

if(Drivers_GPS_NMEA)
  add_executable(GpsNmea gps_nmea.cpp)
  target_link_libraries(GpsNmea ${RACK_LIBS})
  install(TARGETS GpsNmea RUNTIME DESTINATION bin)
endif(Drivers_GPS_NMEA)
