find_package(Boost REQUIRED COMPONENTS headers coroutine)
find_package(Threads REQUIRED)

add_subdirectory(FlexCommon)
add_subdirectory(FlexClient)

add_subdirectory(FlexServer)