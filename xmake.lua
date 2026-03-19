-- Hexview XMake build file
add_rules("mode.debug", "mode.release")

-- Hexview main build target
target("hexview", function ()
	set_version("1.0.0")
	set_kind("binary")
	add_files("src/*.c")

	-- dirs
	set_targetdir("build")
	add_includedirs("include")
	add_linkdirs("$(builddir)/libs")

	-- compiler
	set_languages("c11")
	set_toolchains("clang")
	set_warnings("allextra")

	-- defines
	add_defines("DEBUG")

	-- generate compile commands
	after_build( function (target)
		import("core.project.task")

		task.run("project", {
			quiet = true,
			kind = "compile_commands",
			lsp = "clangd",
			outputdir = "build/",
		})

		print("INFO: generated compile commands")
	end)
end)
