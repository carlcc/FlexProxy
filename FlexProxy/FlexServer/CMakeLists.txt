
file(GLOB_RECURSE FLEX_SERVER_SRC_FILES src/* include/*)

add_executable(FlexServer ${FLEX_SERVER_SRC_FILES})
target_include_directories(FlexServer
PUBLIC
    include
    ${Boost_INCLUDE_DIR}
)
target_link_libraries(FlexServer
PUBLIC
    FlexCommon
    ${Boost_LIBRARIES}
    Threads::Threads
    $<TARGET_NAME:jsonmapper>
)
set_target_properties(FlexServer
PROPERTIES
    CXX_STANDARD 17
)