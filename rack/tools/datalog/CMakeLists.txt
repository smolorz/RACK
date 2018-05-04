set(TOOLS_DATALOG_SUBDIR "tools/datalog")
set(TOOLS_DATALOG_INCLUDE_DIR "${PKG_INCLUDE_DIR}/${TOOLS_DATALOG_SUBDIR}")

set(TOOLS_DATALOG_INCLUDE_HEADERS

    datalog_rec_class.h
)

install(FILES ${TOOLS_DATALOG_INCLUDE_HEADERS} DESTINATION ${TOOLS_DATALOG_INCLUDE_DIR})


option(Tools_DatalogRec "Record data from RACK modules" OFF)

if(Tools_DatalogRec)
  add_executable(DatalogRec datalog_rec.cpp)
  target_link_libraries(DatalogRec ${RACK_LIBS})
  install(TARGETS DatalogRec RUNTIME DESTINATION bin)
endif(Tools_DatalogRec)

