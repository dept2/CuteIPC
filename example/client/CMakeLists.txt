CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Qt
FIND_PACKAGE(Qt4 4.4 COMPONENTS QtCore QtNetwork REQUIRED)
INCLUDE(${QT_USE_FILE})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})


SET(sources
  src/main.cpp
  src/TestObjectSlot.cpp
)

SET(moc_headers
  src/TestObjectSlot.h
)

QT4_WRAP_CPP(sources ${moc_headers})

SET(bin_target CuteIPCClient)
ADD_EXECUTABLE(${bin_target} ${sources})
TARGET_LINK_LIBRARIES(${bin_target} ${QT_LIBRARIES} CuteIPC)
