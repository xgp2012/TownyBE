-- Towny Plugin for LeviLamina
add_requires("nlohmann_json")

target("Towny")
    set_kind("shared")
    set_languages("c++20")
    add_defines("MOD_EXPORT")
    add_includedirs("src", {public = true})

    -- LeviLamina API headers - adjust this path to match your system
    add_includedirs("/c/LeviLamina/src", {public = true})
    -- If using vcpkg or conan, the path might be:
    -- add_includedirs("C:/vcpkg/installed/x64-windows/include", {public = true})

    add_files("src/*.cpp", "src/**/*.cpp")
    add_packages("nlohmann_json")
target_end()
