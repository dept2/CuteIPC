CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Project
PROJECT(CuteIPC)

# CMake module path
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CMake)

# Compile flags
INCLUDE(CompilerFlags)

# Qt
FIND_PACKAGE(Qt4 4.4 COMPONENTS QtCore QtGui QtNetwork REQUIRED)
INCLUDE(${QT_USE_FILE})


# Include directories
INCLUDE_DIRECTORIES(
  src
  ${CMAKE_CURRENT_BINARY_DIR}
)


SET(sources
  src/CuteIPCService.cpp
  src/CuteIPCInterface.cpp
  src/CuteIPCMarshaller.cpp
  src/CuteIPCServiceConnection.cpp
  src/CuteIPCInterfaceConnection.cpp
  src/CuteIPCMessage.cpp
  src/CuteIPCSignalHandler.cpp
)

SET(headers
  src/CuteIPCService_p.h
  src/CuteIPCInterface_p.h
  src/CuteIPCMarshaller_p.h
  src/CuteIPCMessage_p.h
  src/CuteIPCSignalHandler_p.h
)

SET(moc_headers
  src/CuteIPCServiceConnection_p.h
  src/CuteIPCInterfaceConnection_p.h
)

QT4_WRAP_CPP(sources src/CuteIPCService.h OPTIONS -f${CMAKE_CURRENT_SOURCE_DIR}/src/CuteIPCService_p.h)
QT4_WRAP_CPP(sources src/CuteIPCInterface.h OPTIONS -f${CMAKE_CURRENT_SOURCE_DIR}/src/CuteIPCInterface_p.h)
QT4_WRAP_CPP(sources ${moc_headers})

SET(lib_target CuteIPC)
ADD_LIBRARY(${lib_target} SHARED ${sources} ${headers} ${moc_headers})
TARGET_LINK_LIBRARIES(${lib_target} ${QT_LIBRARIES})


# Examples
ADD_SUBDIRECTORY(example/client)
ADD_SUBDIRECTORY(example/server)
