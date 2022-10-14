
require "tundra.syntax.glob"
require "tundra.syntax.files"
local native = require "tundra.native"

ExternalLibrary {
    Name = "defaultConfiguration",
    Pass = "CompileGenerator",
    Propagate = {
        Libs = {Config="win32-*-*"; "User32.lib", "Gdi32.lib", "Ws2_32.lib", "shell32.lib" },
        Libs = {Config="linux_x86-*-*"; "pthread", "dl" },
    },
}  

--============================================================================================--  

Program {
   Name = "repotool",
   Sources = {
      "main.cpp",
      "eastlglue.cpp",
      "callexecutable.cpp",
      "workersystem.cpp",
      "cachedrepositoryio.cpp",
      "report.cpp",
      "repostateio.cpp",

      "commands/checkforupdatesonserver.cpp",
      "commands/scandirectories.cpp",
      "commands/savestate.cpp",
      "commands/pullrepositories.cpp",
      "commands/generateupdatereport.cpp",
      "commands/exportrepositories.cpp",
      "commands/mergelist.cpp",

      "extlibs/reproc/src/c/common.c",
      {Config="win32-*-*"; "extlibs/reproc/src/c/windows/handle.c"},
      {Config="win32-*-*"; "extlibs/reproc/src/c/windows/pipe.c"},
      {Config="win32-*-*"; "extlibs/reproc/src/c/windows/process.c"},
      {Config="win32-*-*"; "extlibs/reproc/src/c/windows/reproc.c"},
      {Config="win32-*-*"; "extlibs/reproc/src/c/windows/string_utils.c"},
      {Config="linux_x86-*-*"; "extlibs/reproc/src/c/posix/fork.c"},
      {Config="linux_x86-*-*"; "extlibs/reproc/src/c/posix/pipe.c"},
      {Config="linux_x86-*-*"; "extlibs/reproc/src/c/posix/process.c"},
      {Config="linux_x86-*-*"; "extlibs/reproc/src/c/posix/reproc.c"},

      "extlibs/anyoption/anyoption.cpp",

--      "extlibs/eacoreallocator/newdelete.cpp",
   },
   Depends = { 
      "eastl", "defaultConfiguration"
   },
   Env = {
      CPPPATH = {       -- keep in sync with .clang_complete file
         ".",
         "extlibs", 
         "extlibs/reproc/include/c",
      },
   },
}
Default "repotool"
