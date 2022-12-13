
target("asio")
    set_kind("headeronly")
    add_includedirs("include", { interface = true })
    add_headerfiles("include/**.h", "include/**.hpp")
    add_defines("ASIO_STANDALONE", { public = true })
target_end()