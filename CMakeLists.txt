cmake_minimum_required(VERSION 3.20)

project(MumbleShare VERSION 0.1.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(WIN32)
    enable_language(RC)

    function(add_windows_version_resource target file_type original_filename file_description)
        set(WINDOWS_FILE_TYPE "${file_type}")
        set(WINDOWS_ORIGINAL_FILENAME "${original_filename}")
        set(WINDOWS_FILE_DESCRIPTION "${file_description}")
        set(resource_file "${CMAKE_CURRENT_BINARY_DIR}/${target}-version.rc")
        configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/windows/version.rc.in"
            "${resource_file}"
            @ONLY
        )
        target_sources("${target}" PRIVATE "${resource_file}")
    endfunction()
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

add_library(mumble_share_protocol STATIC
    config/ServiceUrl.cpp
    protocol/Invitation.cpp
)
target_include_directories(mumble_share_protocol PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(mumble-share-helper
    helper/main.cpp
)
target_link_libraries(mumble-share-helper PRIVATE mumble_share_protocol)
if(WIN32)
    add_windows_version_resource(
        mumble-share-helper VFT_APP
        "mumble-share-helper.exe"
        "Mumble Share browser and invitation helper"
    )
    set_target_properties(mumble-share-helper PROPERTIES WIN32_EXECUTABLE TRUE)
    target_link_libraries(mumble-share-helper PRIVATE shell32 user32)
    target_compile_definitions(mumble-share-helper PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
    if(MINGW)
        target_link_options(mumble-share-helper PRIVATE -static -static-libgcc -static-libstdc++)
    endif()
endif()

option(MUMBLE_SHARE_BUILD_PLUGIN "Build the Mumble client plugin" ON)
if(MUMBLE_SHARE_BUILD_PLUGIN)
    set(MUMBLE_PLUGIN_CPP_DIR "" CACHE PATH "Path to the official mumble-plugin-cpp source tree")
    if(NOT EXISTS "${MUMBLE_PLUGIN_CPP_DIR}/CMakeLists.txt")
        message(FATAL_ERROR "Set MUMBLE_PLUGIN_CPP_DIR to a mumble-plugin-cpp checkout")
    endif()

    set(server-event-callbacks ON CACHE BOOL "" FORCE)
    set(plugin-data-callbacks ON CACHE BOOL "" FORCE)
    set(key-event-callbacks ON CACHE BOOL "" FORCE)
    set(plugin-update ON CACHE BOOL "" FORCE)
    add_subdirectory("${MUMBLE_PLUGIN_CPP_DIR}" "${CMAKE_BINARY_DIR}/mumble-plugin-cpp")

    add_library(mumble-share MODULE
        plugin/Plugin.cpp
        plugin/ProcessLauncher.cpp
    )
    set_target_properties(mumble-share PROPERTIES
        PREFIX ""
        OUTPUT_NAME "mumble-share"
    )
    target_include_directories(mumble-share PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(mumble-share PRIVATE
        "$<LINK_LIBRARY:WHOLE_ARCHIVE,mumble_plugin_cpp_wrapper>"
        mumble_share_protocol
    )
    if(WIN32)
        add_windows_version_resource(
            mumble-share VFT_DLL
            "mumble-share.dll"
            "Mumble Share plugin for Mumble"
        )
        target_link_libraries(mumble-share PRIVATE advapi32 shell32 user32)
        target_compile_definitions(mumble-share PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
        if(MINGW)
            target_link_options(mumble-share PRIVATE -static -static-libgcc -static-libstdc++)
        endif()
    endif()
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(protocol-tests tests/InvitationTests.cpp)
    target_link_libraries(protocol-tests PRIVATE mumble_share_protocol)
    add_test(NAME invitation-protocol COMMAND protocol-tests)

    add_executable(service-url-tests tests/ServiceUrlTests.cpp)
    target_link_libraries(service-url-tests PRIVATE mumble_share_protocol)
    add_test(NAME service-url-configuration COMMAND service-url-tests)
endif()
