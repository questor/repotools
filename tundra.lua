
local common = {
	Env = {
    SHADERCOMPILER = "$(OBJECTDIR)$(SEP)shaderc$(PROGSUFFIX)", 
		--GENERATE_PDB = {
			--{ Config = "*-msvc-release"; "0" },
			--{ Config = "*-msvc-debug"; "1" },
		--},
  		CXXOPTS = {
        {Config="linux_x86-*-*"; "-std=c++11"},
        {Config="linux_x86-*-debug"; "-g", "-O0", "-fno-omit-frame-pointer"},
        {Config="win32-msvc-*"; "/FS" },
      },
      CCOPTS = {
        {Config="win32-msvc-*"; "/FS" },
      },
      CPPDEFS = {
        {Config="*-*-debug"; "DEBUG"},
        {Config="win32-*-*"; "_WIN32", "_X86_", "_WIN32_WINNT=0x0600"},
        {"ENTRY_CONFIG_IMPLEMENT_MAIN=1", "EASTL_NOMINMAX", "EASTL_MINMAX_ENABLED", "EASTL_MOVE_SEMANTICS_ENABLED", "EA_HAVE_CPP11_INITIALIZER_LIST=1", "_CHAR16T"},
      },
      GENERATE_PDB = {
         "1"
      },
      PROTOCOMPILER = "$(OBJECTDIR)$(SEP)protoc$(PROGSUFFIX)", 
	},
}

_G.LIBROOT_EASTL = "extlibs/eastl"

Build {
  IdeGenerationHints = {
    QtCreator = {
      SolutionName = "_REPOTOOL.pro",
    },
  },
	Passes = {
		CompileGenerator = { Name="Compile generator", BuildOrder = 1 },
		CodeGeneration = { Name="Generate sources", BuildOrder = 2 },
	},
	Units = {
      "extlibs/eastl/library.lua",
      "units.lua",
   	},
	Configs = {
		{
			Name = "macosx-gcc",
			DefaultOnHost = "macosx",
			Tools = { "gcc" },
			Inherit = common,
			ReplaceEnv = {
				LD = {Config = { "*-gcc-*" }; "$(CXX)"},
			},
		},
      {
         Name = "linux_x86-gcc",
         DefaultOnHost = "linux",
         Tools = { "gcc" },
         Inherit = common,
         SupportedHosts = { "linux" },
         ReplaceEnv = {
            -- Link with the C++ compiler to get the C++ standard library.
            LD = "$(CXX)",
         },
      },
--      {
--         Name = "linux_x86-clang",
--         --DefaultOnHost = "linux",
--         Tools = { "gcc" },
--         Inherit = common,
--         SupportedHosts = { "linux" },
--         ReplaceEnv = {
--            CC = "clang",
--            CXX = "clang++",
--            LD = "clang",
--         },
--      },
		{
			Name = "win32-msvc",
			DefaultOnHost = "windows",
			Tools = { "msvc" },
			Inherit = common,
		},
	},
}
