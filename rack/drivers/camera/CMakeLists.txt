option(Drivers/Camera_V4L "Camera - V4l" OFF)
option(Drivers/Camera_DCAM "Camera - Dcam" OFF)
option(Drivers/Camera_JPEG "Camera - JPEG" OFF)

if(RACK_CAMERA_V4L)
  add_executable(CameraV4l camera_v4l.cpp)
  target_link_libraries(CameraV4l ${RACK_LIBS})
  install(TARGETS CameraV4l RUNTIME DESTINATION bin)
endif(RACK_CAMERA_V4L)

