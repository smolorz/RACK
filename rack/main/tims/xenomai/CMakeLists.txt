add_executable(tims_client tims_client.c)
target_link_libraries(tims_client pthread)
install(TARGETS tims_client RUNTIME DESTINATION bin)
