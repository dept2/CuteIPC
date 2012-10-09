CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Qt
FIND_PACKAGE(Qt4 4.4 COMPONENTS QtCore QtGui QtNetwork QtTest REQUIRED)
INCLUDE(${QT_USE_FILE})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})


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

QT4_WRAP_CPP(sources ${moc_headers})
QT4_ADD_RESOURCES(sources ${resources})

MACRO(a_test testname testsrc testhdr)
    SET(test_${testname}_SRCS ${testsrc})

    # qt4_automoc(${test_${testname}_SRCS})
    QT4_WRAP_CPP(sources_${testname} ${testhdr})
    add_executable(test_${testname} ${test_${testname}_SRCS} ${sources_${testname}} ${sources})
    target_link_libraries(test_${testname} ${QT_LIBRARIES} CuteIPC)
    ADD_TEST(test_${testname} test_${testname})
ENDMACRO(a_test)

a_test(TestMessageMarshalling src/TestMessageMarshalling.cpp src/TestMessageMarshalling.h)
a_test(TestSignalHandler src/TestSignalHandler.cpp src/TestSignalHandler.h)
a_test(TestSocketCommunication src/TestSocketCommunication.cpp src/TestSocketCommunication.h)
a_test(TestErrorsHandling src/TestErrorsHandling.cpp src/TestErrorsHandling.h)
