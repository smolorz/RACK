add_subdirectory(tims)
add_subdirectory(defines)
add_subdirectory(lib)

set(MAIN_SUBDIR "main")
set(MAIN_INCLUDE_DIR "${PKG_INCLUDE_DIR}/${MAIN_SUBDIR}")

set(MAIN_INCLUDE_HEADERS

    angle_tool.h
    argopts.h
    camera_tool.h
    compress_tool.h
    can_port.h
    dxf_map.h
    pilot_tool.h
    position_tool.h
    rack_byteorder.h
    rack_bits.h
    rack_list_head.h
    rack_gdos.h
    rack_mailbox.h
    rack_mutex.h
    rack_module.h
    rack_data_module.h
    rack_name.h
    rack_proxy.h
    rack_time.h
    rack_task.h
    serial_port.h
    scan3d_compress_tool.h
)

if(Build/ExternalDependencies_libjpeg)
  set(MAIN_INCLUDE_HEADERS ${MAIN_INCLUDE_HEADERS} jpeg_tool.h)
endif(Build/ExternalDependencies_libjpeg)

install(FILES ${MAIN_INCLUDE_HEADERS} DESTINATION ${MAIN_INCLUDE_DIR})

if(NOT Build_OS_Xenomai)
  add_subdirectory(linux)
endif()


if(Build_RACK_Java)

  set(RACK_JAVA_SRC_FILES ${RACK_JAVA_SRC_FILES}

      ${MAIN_SUBDIR}/AngleTool.java
      ${MAIN_SUBDIR}/ContDataMsg.java
      ${MAIN_SUBDIR}/CompressTool.java
      ${MAIN_SUBDIR}/GDOSDataMsg.java
      ${MAIN_SUBDIR}/GDOS.java
      ${MAIN_SUBDIR}/GetDataMsg.java
      ${MAIN_SUBDIR}/GetContDataMsg.java
      ${MAIN_SUBDIR}/RackParam.java
      ${MAIN_SUBDIR}/RackParamMsg.java
      ${MAIN_SUBDIR}/RackProxy.java
      ${MAIN_SUBDIR}/RackDataProxy.java
      ${MAIN_SUBDIR}/RackName.java
      ${MAIN_SUBDIR}/Scan3dCompressTool.java
      ${MAIN_SUBDIR}/StopContDataMsg.java

  PARENT_SCOPE)

endif(Build_RACK_Java)

