-- stealy wheely :* https://github.com/XLabsProject/s1x-client/blob/develop/premake5.lua

-- gitVersioningCommand = "git describe --tags --dirty --always"
-- gitCurrentBranchCommand = "git symbolic-ref -q --short HEAD"

-- Quote the given string input as a C string
function cstrquote(value)
	if value == nil then
		return "\"\""
	end
	result = value:gsub("\\", "\\\\")
	result = result:gsub("\"", "\\\"")
	result = result:gsub("\n", "\\n")
	result = result:gsub("\t", "\\t")
	result = result:gsub("\r", "\\r")
	result = result:gsub("\a", "\\a")
	result = result:gsub("\b", "\\b")
	result = "\"" .. result .. "\""
	return result
end

-- Converts tags in "vX.X.X" format and given revision number Y to an array of numbers {X,X,X,Y}.
-- In the case where the format does not work fall back to padding with zeroes and just ending with the revision number.
-- partscount can be either 3 or 4.
function vertonumarr(value, vernumber, partscount)
	vernum = {}
	for num in string.gmatch(value or "", "%d+") do
		if #vernum < 3 then
			table.insert(vernum, tonumber(num))
		end
	end
	while #vernum < 3 do
		table.insert(vernum, 0)
	end
	if #vernum < partscount then
		table.insert(vernum, tonumber(vernumber))
	end
	return vernum
end

-----------------------------------------------------------------------------

dependencies = {
	basePath = "./deps"
}

function dependencies.load()
	dir = path.join(dependencies.basePath, "premake/*.lua")
	deps = os.matchfiles(dir)

	for i, dep in pairs(deps) do
		dep = dep:gsub(".lua", "")
		require(dep)
	end
end

function dependencies.imports()
	for i, proj in pairs(dependencies) do
		if type(i) == 'number' then
			proj.import()
		end
	end
end

function dependencies.projects()
	for i, proj in pairs(dependencies) do
		if type(i) == 'number' then
			proj.project()
		end
	end
end

dependencies.load()

workspace "wawtool"

	startproject "wawtool"
	location "./build"
	objdir "%{wks.location}/obj"
	targetdir "%{wks.location}/bin/%{cfg.buildcfg}"
	
    configurations { 
        "Debug", 
        "Release" 
    }

	platforms "Win32"
	architecture "x86"

    buildoptions "/std:c++latest"
	systemversion "latest"
    symbols "On"
    staticruntime "On"

    disablewarnings {
		"4239",
		"4505",
		"4996",
		"6001",
		"6385",
		"6386",
		"26812"
	}

    defines { 
        "_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS" 
    }

    filter "platforms:Win*"
		defines {"_WINDOWS", "WIN32"}
	filter {}

	-- Release

	filter "configurations:Release"
		optimize "Full"
		buildoptions {"/GL"}
		defines {"NDEBUG"}
		
		flags { 
            "MultiProcessorCompile", 
            "LinkTimeOptimization", 
            "No64BitChecks",
			"FatalCompileWarnings"
        }
	filter {}

	-- Debug

	filter "configurations:Debug"
		optimize "Debug"

		defines { 
            "DEBUG", 
            "_DEBUG" 
        }

		flags { 
            "MultiProcessorCompile", 
            "No64BitChecks" 
        }
	filter {}

	-- Project

	project "wawtool"
		kind "SharedLib"
		language "C++"

		linkoptions {
			"/PDBCompress"
		}

		pchheader "std_include.hpp"
		pchsource "src/std_include.cpp"

		files {
			"./src/**.rc",
			"./src/**.hpp",
			"./src/**.cpp",
		}

		includedirs {
			"%{prj.location}/src",
			"./src",
		}

		resincludedirs {
			"$(ProjectDir)src"
		}

        buildoptions { 
            "/Zm100 -Zm100" 
        }

		-- if(os.getenv("COD5_ROOT")) then
			-- --[[ print ("Setup paths using environment variable 'COD5_ROOT' :: '" .. os.getenv("COD5_ROOT") .. "'") ]]
			-- targetdir(os.getenv("COD5_ROOT"))
			-- debugdir (os.getenv("COD5_ROOT"))
			-- debugcommand (os.getenv("COD5_ROOT") .. "/CoDWaWmp.exe")
		-- end

        -- Specific configurations
		flags { "UndefinedIdentifiers" }
		warnings "Extra"

		
		-- -- Pre-build
		-- prebuildcommands {
			-- "pushd %{_MAIN_SCRIPT_DIR}",
			-- "tools\\premake5 generate-buildinfo",
			-- "popd",
		-- }

		-- Post-build
		postbuildcommands {
			"echo ----------------------------",
			"echo Rename \"wawtool.dll\" to \"wawtool.asi\"",
			"MOVE /Y \"$(TargetDir)wawtool.dll\" \"$(TargetDir)wawtool.asi\"",
		}

		dependencies.imports()

        group "Dependencies"
            dependencies.projects()

