add_subdirectory(scan2d)
#add_subdirectory(obj_recog)

set(PERCEPTION_SUBDIR "perception")
set(PERCEPTION_INCLUDE_DIR "${PKG_INCLUDE_DIR}/${PERCEPTION_SUBDIR}")

set(PERCEPTION_INCLUDE_HEADERS

    obj_recog_proxy.h
    scan2d_proxy.h
    scan3d_proxy.h
)


install(FILES ${PERCEPTION_INCLUDE_HEADERS} DESTINATION ${PERCEPTION_INCLUDE_DIR})


if(Build_RACK_Java)

  set(RACK_JAVA_SRC_FILES ${RACK_JAVA_SRC_FILES}
 
      ${PERCEPTION_SUBDIR}/ObjRecogProxy.java
      ${PERCEPTION_SUBDIR}/ObjRecogDataMsg.java
      ${PERCEPTION_SUBDIR}/Scan2dProxy.java
      ${PERCEPTION_SUBDIR}/Scan2dDataMsg.java
      ${PERCEPTION_SUBDIR}/Scan3dProxy.java
      ${PERCEPTION_SUBDIR}/Scan3dDataMsg.java
      ${PERCEPTION_SUBDIR}/Scan3dRangeImageMsg.java

  PARENT_SCOPE)

endif(Build_RACK_Java)

