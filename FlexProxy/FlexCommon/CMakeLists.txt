
file(GLOB_RECURSE FLEX_COMMON_SRC_FILES src/* include/*)

add_library(FlexCommon STATIC ${FLEX_COMMON_SRC_FILES})
target_include_directories(FlexCommon
PUBLIC
    include
    ${Boost_INCLUDE_DIR}
)
target_link_libraries(FlexCommon
PUBLIC
    ${Boost_LIBRARIES}
)
set_target_properties(FlexCommon
PROPERTIES
    CXX_STANDARD 17
)