target("FlexServer")
    set_kind("binary")
    add_files("src/**.cpp")
    add_headerfiles("include/**")
    add_includedirs("include")
    set_languages("cxx20")

    add_deps("FlexCommon", "jsonmapper", "asio")
target_end()