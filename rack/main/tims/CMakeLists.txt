add_subdirectory(router)

if(Build_OS_Xenomai)
  add_subdirectory(xenomai)
  if(Build_KMod)
    add_subdirectory(xenomai_kmod)
  endif(Build_KMod)
endif(Build_OS_Xenomai)

set(MAIN_TIMS_SUBDIR "main/tims")
set(TIMS_INCLUDE_DIR "${PKG_INCLUDE_DIR}/${MAIN_TIMS_SUBDIR}")

set(TIMS_INCLUDE_HEADERS

    tims.h
    tims_api.h
    tims_byteorder.h
    tims_router.h
    tims_rtdm.h
    tims_types.h
)

install(FILES ${TIMS_INCLUDE_HEADERS} DESTINATION ${TIMS_INCLUDE_DIR})


if(Build_RACK_Java)

  set(RACK_JAVA_SRC_FILES ${RACK_JAVA_SRC_FILES}

      ${MAIN_TIMS_SUBDIR}/BigEndianDataInputStream.java
      ${MAIN_TIMS_SUBDIR}/EndianDataInputStream.java
      ${MAIN_TIMS_SUBDIR}/LittleEndianDataInputStream.java
      ${MAIN_TIMS_SUBDIR}/Tims.java
      ${MAIN_TIMS_SUBDIR}/TimsDataRate.java
      ${MAIN_TIMS_SUBDIR}/TimsRawMsg.java
      ${MAIN_TIMS_SUBDIR}/TimsMbx.java
      ${MAIN_TIMS_SUBDIR}/TimsMsg.java
      ${MAIN_TIMS_SUBDIR}/TimsRouter.java
      ${MAIN_TIMS_SUBDIR}/TimsRouterMbxMsg.java
      ${MAIN_TIMS_SUBDIR}/TimsTcp.java
      ${MAIN_TIMS_SUBDIR}/TimsException.java
      ${MAIN_TIMS_SUBDIR}/TimsTimeoutException.java

  PARENT_SCOPE)

endif(Build_RACK_Java)

