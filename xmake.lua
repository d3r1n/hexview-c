-- Hexview XMake build file
add_rules("mode.debug", "mode.release")

if is_mode("debug") then
    add_defines("DEBUG")
end

-- Global configurations inherited by all targets
set_languages("c11")
set_toolchains("clang")
set_warnings("allextra")
set_targetdir("build")
add_includedirs("include")
add_linkdirs("$(builddir)/libs")

-- 1. Hexview main application binary
target("hexview")
    set_version("1.0.0")
    set_kind("binary")
    add_files("src/*.c")

-- 2. Dynamically register all test files under test/*.c
-- os.files returns an array of all matching file paths
for _, filepath in ipairs(os.files("test/*.c")) do

    -- Extract the file name without extension to use as the target name
    -- e.g., "test/test_string.c" -> "test_string"
    local name = path.basename(filepath)

    target(name)
        set_kind("binary")

        -- Add the implementation files (excluding the main application entry point)
        add_files("src/*.c")
        remove_files("src/main.c")

        -- Add this specific test file
        add_files(filepath)

        -- Register it with XMake's testing engine
        add_tests(name)
end
