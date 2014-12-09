import qbs 1.0

Project {
  DynamicLibrary {
    name: "CuteIPC"

    files: [ "src/*", "include/*" ]

    Depends { name: "cpp" }

    //cpp.rpaths: cpp.libraryPaths
    cpp.includePaths: "include"
    cpp.defines: "CUTEIPC_LIBRARY"

    Depends {
      name: "Qt";
      submodules: [ "core", "gui", "network" ]
    }

    Export {
      Depends { name: "cpp" }
      cpp.includePaths: "include"
    }
  }

  references: [
    "test/test.qbs",
  ]
}
