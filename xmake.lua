set_project("FlexProxy")

if is_plat("windows") then
    add_cxxflags("/Zc:__cplusplus") -- 似乎默认没给加 此选项，
end

includes("Thirdparty/JsonMapper")
includes("FlexProxy")