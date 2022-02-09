
file(GLOB_RECURSE FLEX_SERVER_SRC_FILES src/* include/*)

add_executable(FlexClient ${FLEX_SERVER_SRC_FILES})
target_include_directories(FlexClient
PUBLIC
    include
    ${Boost_INCLUDE_DIR}
)
target_link_libraries(FlexClient
PUBLIC
    FlexCommon
    ${Boost_LIBRARIES}
    Threads::Threads
    $<TARGET_NAME:jsonmapper>
)
set_target_properties(FlexClient
PROPERTIES
    CXX_STANDARD 17
)