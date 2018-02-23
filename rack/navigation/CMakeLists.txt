add_subdirectory(pilot)
add_subdirectory(position)
add_subdirectory(odometry)

set(NAVIGATION_SUBDIR "navigation")
set(NAVIGATION_INCLUDE_DIR "${PKG_INCLUDE_DIR}/${NAVIGATION_SUBDIR}")

set(NAVIGATION_INCLUDE_HEADERS

    feature_map_proxy.h
    grid_map_proxy.h
    mcl_proxy.h
    odometry_proxy.h
    path_proxy.h
    pilot_proxy.h
    position_proxy.h
)

install(FILES ${NAVIGATION_INCLUDE_HEADERS} DESTINATION ${NAVIGATION_INCLUDE_DIR})


if(Build_RACK_Java)

  set(RACK_JAVA_SRC_FILES ${RACK_JAVA_SRC_FILES}

      ${NAVIGATION_SUBDIR}/FeatureMapDataMsg.java
      ${NAVIGATION_SUBDIR}/FeatureMapDataPoint.java
      ${NAVIGATION_SUBDIR}/FeatureMapFeatureMsg.java
      ${NAVIGATION_SUBDIR}/FeatureMapFilenameMsg.java
      ${NAVIGATION_SUBDIR}/FeatureMapProxy.java
      ${NAVIGATION_SUBDIR}/GridMapProxy.java
      ${NAVIGATION_SUBDIR}/GridMapDataMsg.java
      ${NAVIGATION_SUBDIR}/MclProxy.java
      ${NAVIGATION_SUBDIR}/MclDataMsg.java
      ${NAVIGATION_SUBDIR}/MclFilenameMsg.java
      ${NAVIGATION_SUBDIR}/MclDataPoint.java
      ${NAVIGATION_SUBDIR}/OdometryProxy.java
      ${NAVIGATION_SUBDIR}/OdometryDataMsg.java
      ${NAVIGATION_SUBDIR}/PathProxy.java
      ${NAVIGATION_SUBDIR}/PathDataMsg.java
      ${NAVIGATION_SUBDIR}/PathDestMsg.java
      ${NAVIGATION_SUBDIR}/PathMakeMsg.java
      ${NAVIGATION_SUBDIR}/PathRddfMsg.java
      ${NAVIGATION_SUBDIR}/PathLayerMsg.java
      ${NAVIGATION_SUBDIR}/PilotProxy.java 
      ${NAVIGATION_SUBDIR}/PilotDataMsg.java
      ${NAVIGATION_SUBDIR}/PilotDestMsg.java
      ${NAVIGATION_SUBDIR}/PilotMultiDestMsg.java
      ${NAVIGATION_SUBDIR}/PilotHoldMsg.java
      ${NAVIGATION_SUBDIR}/PositionDataMsg.java
      ${NAVIGATION_SUBDIR}/PositionProxy.java
      ${NAVIGATION_SUBDIR}/PositionGkDataMsg.java
      ${NAVIGATION_SUBDIR}/PositionUtmDataMsg.java
      ${NAVIGATION_SUBDIR}/PositionWgs84DataMsg.java
      ${NAVIGATION_SUBDIR}/PositionTool.java

  PARENT_SCOPE)

endif(Build_RACK_Java)

