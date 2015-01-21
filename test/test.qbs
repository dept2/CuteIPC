import qbs
Project {

  //1
  CppApplication {
    name: "TestMessageMarshalling"

    files: [ "src/ServiceTestObject.*",
             "src/InterfaceTestObject.*",
             "src/SignalWaiter.*",
             "images/images.qrc",
             "src/TestMessageMarshalling.*"
    ]

    cpp.includePaths: "../src"

    Depends { name: "Qt"; submodules: [ "core", "gui", "network", "test" ] }
    Depends { name: "CuteIPC" }

    Properties {
      condition: qbs.toolchain.contains('gcc')
      cpp.cxxFlags: [ "-std=c++11" ]
    }
  }

  //2
  CppApplication {
    name: "TestSignalHandler"

    files: [ "src/ServiceTestObject.*",
             "src/InterfaceTestObject.*",
             "src/SignalWaiter.*",
             "images/images.qrc",
             "src/TestSignalHandler.*"
    ]

    cpp.includePaths: "../src"

    Depends { name: "Qt"; submodules: [ "core", "gui", "network", "test" ] }
    Depends { name: "CuteIPC" }

    Properties {
      condition: qbs.toolchain.contains('gcc')
      cpp.cxxFlags: [ "-std=c++11" ]
    }
  }

  //3
  CppApplication {
    name: "TestSocketCommunication"

    files: [ "src/ServiceTestObject.*",
             "src/InterfaceTestObject.*",
             "src/SignalWaiter.*",
             "images/images.qrc",
             "src/TestSocketCommunication.*"
    ]

    cpp.includePaths: "../src"

    Depends { name: "Qt"; submodules: [ "core", "gui", "network", "test" ] }
    Depends { name: "CuteIPC" }

    Properties {
      condition: qbs.toolchain.contains('gcc')
      cpp.cxxFlags: [ "-std=c++11" ]
    }
  }

  //4
  CppApplication {
    name: "TestErrorsHandling"

    files: [ "src/ServiceTestObject.*",
             "src/InterfaceTestObject.*",
             "src/SignalWaiter.*",
             "images/images.qrc",
             "src/TestErrorsHandling.*"
    ]

    cpp.includePaths: "../src"

    Depends { name: "Qt"; submodules: [ "core", "gui", "network", "test" ] }
    Depends { name: "CuteIPC" }

    Properties {
      condition: qbs.toolchain.contains('gcc')
      cpp.cxxFlags: [ "-std=c++11" ]
    }
  }

}
