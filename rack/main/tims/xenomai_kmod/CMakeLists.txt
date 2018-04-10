add_custom_target(tims_ko ALL cp -p ${CMAKE_CURRENT_SOURCE_DIR}/*.c
                                    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
                                    ${CMAKE_CURRENT_SOURCE_DIR}/Kbuild
                                    ${CMAKE_CURRENT_BINARY_DIR}

                      COMMAND make -C${Linux_Kernel_SRC}
                                   M=${CMAKE_CURRENT_BINARY_DIR}
                                   CROSS_COMPILE=${CROSS_COMPILE}
                                   ARCH=${CMAKE_SYSTEM_PROCESSOR}
                                   PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}
                                   PROJECT_BINARY_DIR=${PROJECT_BINARY_DIR}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tims.ko DESTINATION lib/rack)
