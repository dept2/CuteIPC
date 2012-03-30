CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Project
PROJECT(CuteIPC)

# CMake module path
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CMake)

# Compile flags
INCLUDE(CompilerFlags)

# Qt
FIND_PACKAGE(Qt4 4.4 COMPONENTS QtCore QtNetwork REQUIRED)
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
)

SET(headers
  src/CuteIPCMarshaller_p.h
)

SET(moc_headers
  src/CuteIPCService.h
  src/CuteIPCInterface.h
  src/CuteIPCServiceConnection_p.h
)

QT4_WRAP_CPP(sources ${moc_headers})

SET(lib_target CuteIPC)
ADD_LIBRARY(${lib_target} SHARED ${sources} ${headers})
TARGET_LINK_LIBRARIES(${lib_target} ${QT_LIBRARIES})


# Examples
ADD_SUBDIRECTORY(example/client)
ADD_SUBDIRECTORY(example/server)
