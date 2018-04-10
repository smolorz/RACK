option(Drivers/Camera_V4L "Camera - V4l" OFF)

option(Drivers/Camera_DCAM "Camera - Dcam" OFF)
if(Drivers/Camera_DCAM)
  if(NOT Build/ExternalDependencies_libdc1394)
    message(FATAL_ERROR "*** Please enable external dependency libdc1394")
  endif(NOT Build/ExternalDependencies_libdc1394)
endif(Drivers/Camera_DCAM)

option(Drivers/Camera_JPEG "Camera - JPEG" OFF)
if(Drivers/Camera_JPEG)
  if(NOT Build/ExternalDependencies_libjpeg)
    message(FATAL_ERROR "*** Please enable external dependency libjpeg")
  endif(NOT Build/ExternalDependencies_libjpeg)
endif(Drivers/Camera_JPEG)


if(Drivers/Camera_V4L)
  add_executable(CameraV4l camera_v4l.cpp)
  target_link_libraries(CameraV4l ${RACK_LIBS})
  install(TARGETS CameraV4l RUNTIME DESTINATION bin)
endif(Drivers/Camera_V4L)

if(Drivers/Camera_DCAM)
  add_executable(CameraDcam camera_dcam.cpp)
  target_include_directories(CameraDcam PRIVATE ${LIBRAW1394_INCLUDE_PATH}
                                                ${LIBDC1394_INCLUDE_PATH})
  target_link_libraries(CameraDcam ${RACK_LIBS} ${LIBRAW1394_LIB}
                                                ${LIBDC1394_LIB})
  install(TARGETS CameraDcam RUNTIME DESTINATION bin)
endif(Drivers/Camera_DCAM)

if(Drivers/Camera_JPEG)
  add_executable(CameraJpeg camera_jpeg.cpp)
  target_include_directories(CameraJpeg PRIVATE ${JPEG_INCLUDE_DIR})
  target_link_libraries(CameraJpeg ${RACK_LIBS} ${JPEG_LIBRARIES})
  install(TARGETS CameraJpeg RUNTIME DESTINATION bin)
endif(Drivers/Camera_JPEG)


