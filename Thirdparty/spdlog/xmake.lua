
target("spdlog")
    set_kind("headeronly")
    add_includedirs("include", { interface = true })
    add_headerfiles("include/**")
target_end()