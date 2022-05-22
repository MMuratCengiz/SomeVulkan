ADD_DEFINITIONS(-DENGINE_NAMESPACE=BlazarEngine)
ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
ADD_DEFINITIONS(-DDEBUG)
ADD_DEFINITIONS(-DG_VERBOSITY=3)
ADD_DEFINITIONS(-DGTEST_LANG_CXX11=1)
ADD_DEFINITIONS(-DROOT_DIR="${PROJECT_SOURCE_DIR}")

IF (WIN32)
    ADD_DEFINITIONS(-D_WIN32=1)
ENDIF()

SET(CMAKE_CXX_STANDARD 17)

IF(${BLAZAR_INSTALL_LIBS})
    SET(BLAZAR_INSTALL_LOCATION ${CMAKE_INSTALL_PREFIX}/ CACHE INTERNAL "BLAZAR_INSTALL_LOCATION")
ELSE()
    IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
        SET(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/BlazarEngine/)
    ELSE()
        SET(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/BlazarEngine/${CMAKE_BUILD_TYPE}/)
    ENDIF()
    SET(BLAZAR_INSTALL_LOCATION ${RUNTIME_OUTPUT_DIRECTORY} CACHE INTERNAL "BLAZAR_INSTALL_LOCATION")
ENDIF()