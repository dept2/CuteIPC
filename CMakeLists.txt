CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Project
PROJECT(CuteIPC)

ENABLE_TESTING(true)

# CMake module path
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CMake)

# Compile flags
INCLUDE(CompilerFlags)

# Qt
MESSAGE("Searching for preferred Qt version...")
IF (DEFINED QT_DESIRED_VERSION)
  IF(QT_DESIRED_VERSION MATCHES 5)
    SET(QT_VERSION_MAJOR 5)
    FIND_PACKAGE(Qt5Core REQUIRED)
    FIND_PACKAGE(Qt5Gui REQUIRED)
    FIND_PACKAGE(Qt5Network REQUIRED)
  ELSE (QT_DESIRED_VERSION MATCHES 5)
    IF (QT_DESIRED_VERSION MATCHES 4)
      FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui QtNetwork REQUIRED)
      INCLUDE(${QT_USE_FILE})
    ELSE (QT_DESIRED_VERSION MATCHES 4)
      MESSAGE(FATAL_ERROR "You must specify the 4th or the 5th version of Qt")
    ENDIF(QT_DESIRED_VERSION MATCHES 4)
  ENDIF (QT_DESIRED_VERSION MATCHES 5)
ELSE (DEFINED QT_DESIRED_VERSION)
  FIND_PACKAGE(Qt5Core QUIET)
  FIND_PACKAGE(Qt5Gui QUIET)
  FIND_PACKAGE(Qt5Network QUIET)
  IF (Qt5Core_FOUND)
    SET(QT_VERSION_MAJOR 5)
  ELSE(Qt5Core_FOUND)
    MESSAGE("Qt 5 not found, searching for Qt4")
    FIND_PACKAGE(Qt4 REQUIRED)
  ENDIF(Qt5Core_FOUND)
ENDIF (DEFINED QT_DESIRED_VERSION)
MESSAGE("Qt version is used: ${QT_VERSION_MAJOR}")

IF (QT_VERSION_MAJOR MATCHES 5)
  # Force cmake 2.8.8 in order to have a decent support of Qt5
  CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
ENDIF()


# Include directories
IF (QT_VERSION_MAJOR MATCHES 5)
  INCLUDE_DIRECTORIES(include src ${CMAKE_CURRENT_BINARY_DIR} ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS})
ELSE()
  INCLUDE_DIRECTORIES(include src ${CMAKE_CURRENT_BINARY_DIR})
ENDIF()

# Turn off automoc
SET(CMAKE_AUTOMOC OFF)

SET(sources
  src/CuteIPCService.cpp
  src/CuteIPCInterface.cpp
  src/CuteIPCMarshaller.cpp
  src/CuteIPCServiceConnection.cpp
  src/CuteIPCInterfaceConnection.cpp
  src/CuteIPCMessage.cpp
  src/CuteIPCSignalHandler.cpp
  src/CuteIPCInterfaceWorker.cpp
)

SET(headers
  include/CuteIPCService.h
  include/CuteIPCInterface.h
  src/CuteIPCService_p.h
  src/CuteIPCInterface_p.h
  src/CuteIPCMarshaller_p.h
  src/CuteIPCMessage_p.h
  src/CuteIPCSignalHandler_p.h
)

SET(moc_headers
  src/CuteIPCServiceConnection_p.h
  src/CuteIPCInterfaceConnection_p.h
  src/CuteIPCInterfaceWorker.h
)

IF (QT_VERSION_MAJOR MATCHES 5)
  QT5_WRAP_CPP(sources ${moc_headers})

  QT5_GENERATE_MOC(include/CuteIPCService.h ${CMAKE_CURRENT_BINARY_DIR}/moc_CuteIPCService.cpp)
  SET_SOURCE_FILES_PROPERTIES(src/CuteIPCService.cpp PROPERTIES OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/moc_CuteIPCService.cpp)

  QT5_GENERATE_MOC(include/CuteIPCInterface.h ${CMAKE_CURRENT_BINARY_DIR}/moc_CuteIPCInterface.cpp)
  SET_SOURCE_FILES_PROPERTIES(src/CuteIPCInterface.cpp PROPERTIES OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/moc_CuteIPCInterface.cpp)
ELSE()
  QT4_WRAP_CPP(sources ${moc_headers})

  QT4_GENERATE_MOC(include/CuteIPCService.h ${CMAKE_CURRENT_BINARY_DIR}/moc_CuteIPCService.cpp)
  SET_SOURCE_FILES_PROPERTIES(src/CuteIPCService.cpp PROPERTIES OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/moc_CuteIPCService.cpp)

  QT4_GENERATE_MOC(include/CuteIPCInterface.h ${CMAKE_CURRENT_BINARY_DIR}/moc_CuteIPCInterface.cpp)
  SET_SOURCE_FILES_PROPERTIES(src/CuteIPCInterface.cpp PROPERTIES OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/moc_CuteIPCInterface.cpp)
ENDIF()

SET(lib_target CuteIPC)
ADD_LIBRARY(${lib_target} SHARED ${sources} ${headers} ${moc_headers})

IF (QT_VERSION_MAJOR MATCHES 5)
  TARGET_LINK_LIBRARIES(${lib_target} ${Qt5Core_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5Network_LIBRARIES})
ELSE()
  TARGET_LINK_LIBRARIES(${lib_target} ${QT_LIBRARIES})
ENDIF()

TARGET_INCLUDE_DIRECTORIES(${lib_target} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

SET(CUTEIPC_BUILD_EXAMPLES OFF CACHE BOOL "Build CuteIPC examples")
SET(CUTEIPC_BUILD_TESTS OFF CACHE BOOL "Build CuteIPC tests")


# Examples
IF (CUTEIPC_BUILD_EXAMPLES)
  ADD_SUBDIRECTORY(example/client)
  ADD_SUBDIRECTORY(example/server)
ENDIF ()

IF (CUTEIPC_BUILD_TESTS)
  ADD_SUBDIRECTORY(test)
ENDIF ()

INSTALL(TARGETS ${lib_target} LIBRARY DESTINATION lib)
