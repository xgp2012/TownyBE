-- Towny Plugin for LeviLamina
add_requires("nlohmann_json")

target("Towny")
    set_kind("shared")
    set_languages("c++20")
    add_defines("MOD_EXPORT")
    add_includedirs("src", {public = true})

    -- Use local mock headers for CI
    add_includedirs("third_party", {public = true})

    add_files("src/*.cpp", "src/**/*.cpp")
    add_packages("nlohmann_json")
target_end()
