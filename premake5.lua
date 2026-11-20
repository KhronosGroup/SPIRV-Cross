project "SPIRV-Cross"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"spirv.hpp",
		"spirv_cfg.cpp",
		"spirv_cross.cpp",
		"spirv_cross_parsed_ir.cpp",
		"spirv_cross_util.cpp",
		"spirv_glsl.cpp",
		"spirv_hlsl.cpp",
		"spirv_parser.cpp"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
