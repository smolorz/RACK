set(MAIN_DEFINES_SUBDIR "main/defines")
set(DEFINES_INCLUDE_DIR "${PKG_INCLUDE_DIR}/${MAIN_DEFINES_SUBDIR}")

set(DEFINES_INCLUDE_HEADERS

    destination_point.h
    image_rect.h
    ladar_point.h
    obj_recog_object.h
    point2d.h
    point3d.h
    polar_spline.h
    position2d.h
    position3d.h
    scan_point.h
    scan3d_range_img_point.h
    waypoint2d.h

)

install(FILES ${DEFINES_INCLUDE_HEADERS} DESTINATION ${DEFINES_INCLUDE_DIR})


if(Build_RACK_Java)

  set(RACK_JAVA_SRC_FILES ${RACK_JAVA_SRC_FILES}

      ${MAIN_DEFINES_SUBDIR}/DestinationPoint.java
      ${MAIN_DEFINES_SUBDIR}/ImageRect.java
      ${MAIN_DEFINES_SUBDIR}/LadarPoint.java
      ${MAIN_DEFINES_SUBDIR}/ObjRecogObject.java
      ${MAIN_DEFINES_SUBDIR}/Point2d.java
      ${MAIN_DEFINES_SUBDIR}/Point3d.java
      ${MAIN_DEFINES_SUBDIR}/PolarSpline.java
      ${MAIN_DEFINES_SUBDIR}/Position2d.java
      ${MAIN_DEFINES_SUBDIR}/Position3d.java
      ${MAIN_DEFINES_SUBDIR}/ScanPoint.java
      ${MAIN_DEFINES_SUBDIR}/Scan3dRangeImagePoint.java
      ${MAIN_DEFINES_SUBDIR}/Waypoint2d.java

  PARENT_SCOPE)

endif(Build_RACK_Java)

