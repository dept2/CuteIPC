CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Qt
FIND_PACKAGE(Qt4 4.4 COMPONENTS QtCore QtGui QtNetwork QtTest REQUIRED)
INCLUDE(${QT_USE_FILE})

MESSAGE("Searching for preferred Qt version...")
IF (DEFINED QT_DESIRED_VERSION)
  IF(QT_DESIRED_VERSION MATCHES 5)
    SET(QT_VERSION_MAJOR 5)
    #SET(CMAKE_INCLUDE_CURRENT_DIR ON)
    FIND_PACKAGE(Qt5Core REQUIRED)
    FIND_PACKAGE(Qt5Gui REQUIRED)
    FIND_PACKAGE(Qt5Network REQUIRED)
    FIND_PACKAGE(Qt5Test REQUIRED)
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
  FIND_PACKAGE(Qt5Test QUIET)
  MESSAGE(${Qt5Core_FOUND})
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
  INCLUDE_DIRECTORIES(include src ${CMAKE_CURRENT_BINARY_DIR} ${Qt5Core_INCLUDE_DIRS}
                      ${Qt5Gui_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS} ${Qt5Test_INCLUDE_DIRS})
ELSE()
  INCLUDE_DIRECTORIES(include src ${CMAKE_CURRENT_BINARY_DIR})
ENDIF()


SET(sources
  src/ServiceTestObject.cpp
  src/InterfaceTestObject.cpp
  src/SignalWaiter.cpp
)

SET(moc_headers
  src/ServiceTestObject.h
  src/InterfaceTestObject.h
  src/SignalWaiter.h
)

SET(resources
  images/images.qrc
)

IF (QT_VERSION_MAJOR MATCHES 5)
  QT5_WRAP_CPP(sources ${moc_headers})
  QT5_ADD_RESOURCES(sources ${resources})
ELSE()
  QT4_WRAP_CPP(sources ${moc_headers})
  QT4_ADD_RESOURCES(sources ${resources})
ENDIF()

MACRO(a_test testname testsrc testhdr)
    SET(test_${testname}_SRCS ${testsrc})

    IF (QT_VERSION_MAJOR MATCHES 5)
      QT5_WRAP_CPP(sources_${testname} ${testhdr})
      ADD_EXECUTABLE(test_${testname} ${test_${testname}_SRCS} ${sources_${testname}} ${sources})
      TARGET_LINK_LIBRARIES(test_${testname} ${Qt5Core_LIBRARIES} ${Qt5Gui_LIBRARIES}
                            ${Qt5Network_LIBRARIES} ${Qt5Test_LIBRARIES} CuteIPC)
    ELSE()
      QT4_WRAP_CPP(sources_${testname} ${testhdr})
      ADD_EXECUTABLE(test_${testname} ${test_${testname}_SRCS} ${sources_${testname}} ${sources})
      TARGET_LINK_LIBRARIES(test_${testname} ${QT_LIBRARIES} CuteIPC)
    ENDIF()

    ADD_TEST(test_${testname} test_${testname})
ENDMACRO(a_test)

a_test(TestMessageMarshalling src/TestMessageMarshalling.cpp src/TestMessageMarshalling.h)
a_test(TestSignalHandler src/TestSignalHandler.cpp src/TestSignalHandler.h)
a_test(TestSocketCommunication src/TestSocketCommunication.cpp src/TestSocketCommunication.h)
a_test(TestErrorsHandling src/TestErrorsHandling.cpp src/TestErrorsHandling.h)
