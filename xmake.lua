set_project("FlexProxy")

if is_plat("windows") then
    add_cxxflags("/Zc:__cplusplus") -- 似乎默认没给加 此选项，
end

includes("Thirdparty/asio-1.24.0")
includes("Thirdparty/spdlog")
includes("Thirdparty/JsonMapper")
includes("FlexProxy")