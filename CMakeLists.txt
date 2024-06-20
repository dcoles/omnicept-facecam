cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

project(omnicept-facecam C CXX)

set(HP_OMNICEPT_SDK_INSTALL "$ENV{HP_OMNICEPT_INSTALL}" CACHE STRING "Install location of the HP Omnicept SDK.")
list(APPEND CMAKE_MODULE_PATH "${HP_OMNICEPT_SDK_INSTALL}/cmake")
find_package(HPOmnicept REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 17)

set(target "main")
add_executable(${target}
    src/main.cpp
)

target_link_libraries(${target} PRIVATE HPOmnicept::lib)

add_custom_command(TARGET ${target} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:HPOmnicept::jsoncpp> $<TARGET_FILE_DIR:${target}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:HPOmnicept::zeromq> $<TARGET_FILE_DIR:${target}>
)
