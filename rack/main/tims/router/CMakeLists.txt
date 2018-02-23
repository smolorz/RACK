add_executable(tims_router_tcp tims_router_tcp.c)
target_link_libraries(tims_router_tcp pthread)
install(TARGETS tims_router_tcp RUNTIME DESTINATION bin)
