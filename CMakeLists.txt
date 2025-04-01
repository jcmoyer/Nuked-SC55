cmake_minimum_required (VERSION 3.19)
project (nuked-sc55 VERSION 0.5.0 LANGUAGES C CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(GNUInstallDirs)
include(TargetWarnings)

set(NUKED_SOURCE "unknown" CACHE STRING "Origin of this build")

#==============================================================================
# Tests
#==============================================================================
option(NUKED_ENABLE_TESTS "Enable tests" OFF)
set(NUKED_TEST_ROMDIR "" CACHE PATH
    "Directory the test runner should look for roms in")

if(NUKED_ENABLE_TESTS)
    enable_testing()
    add_subdirectory("test")
endif()

#==============================================================================
# Dependencies
#==============================================================================
find_package(SDL2)

if(SDL2_FOUND)
    message(STATUS "SDL2 FOUND - building standard frontend")
else()
    message(STATUS "SDL2 NOT FOUND - the standard frontend will not be built!")
endif()

if(NOT WIN32)
    set(USE_RTMIDI TRUE)
endif()

if(USE_RTMIDI)
    find_package(rtmidi MODULE REQUIRED)
endif()

# ASIO
if(WIN32)
    option(NUKED_ENABLE_ASIO "Enable ASIO" OFF)
    set(NUKED_ASIO_SDK_DIR "" CACHE PATH
        "Directory containing the ASIO SDK")
endif()

#==============================================================================
# Backend
#==============================================================================
configure_file(src/backend/config.h.in backend/config.h @ONLY)
add_library(nuked-sc55-backend)
target_sources(nuked-sc55-backend
    PRIVATE
    src/backend/config.cpp
    src/backend/emu.cpp
    src/backend/lcd.cpp
    src/backend/mcu.cpp
    src/backend/mcu_interrupt.cpp
    src/backend/mcu_opcodes.cpp
    src/backend/mcu_timer.cpp
    src/backend/path_util.cpp
    src/backend/pcm.cpp
    src/backend/submcu.cpp

    src/backend/sha/sha-private.h
    src/backend/sha/sha.h
    src/backend/sha/sha224-256.c

    PUBLIC
    FILE_SET public_headers
    TYPE HEADERS
    BASE_DIRS src "${CMAKE_CURRENT_BINARY_DIR}"
    FILES
    "${CMAKE_CURRENT_BINARY_DIR}/backend/config.h"
    src/backend/audio.h
    src/backend/cast.h
    src/backend/command_line.h
    src/backend/emu.h
    src/backend/lcd.h
    src/backend/lcd_back.h
    src/backend/lcd_font.h
    src/backend/math_util.h
    src/backend/mcu.h
    src/backend/mcu_interrupt.h
    src/backend/mcu_opcodes.h
    src/backend/mcu_timer.h
    src/backend/path_util.h
    src/backend/pcm.h
    src/backend/ringbuffer.h
    src/backend/submcu.h
)
target_include_directories(nuked-sc55-backend PUBLIC "src/backend" "${CMAKE_CURRENT_BINARY_DIR}/backend")
target_compile_features(nuked-sc55-backend PRIVATE cxx_std_23)
target_enable_warnings(nuked-sc55-backend)

#==============================================================================
# Standard frontend
#==============================================================================
if(SDL2_FOUND)
    add_executable(nuked-sc55)
    target_sources(nuked-sc55
        PRIVATE
        src/standard/audio_sdl.cpp
        src/standard/lcd_sdl.cpp
        src/standard/main.cpp
        src/standard/midi.h
        src/standard/output_sdl.cpp

        PRIVATE FILE_SET headers TYPE HEADERS FILES
        src/standard/audio_sdl.h
        src/standard/lcd_sdl.h
        src/standard/output_sdl.h
    )

    if(USE_RTMIDI)
        target_sources(nuked-sc55 PRIVATE src/standard/midi_rtmidi.cpp)
    elseif(WIN32)
        target_sources(nuked-sc55 PRIVATE src/standard/midi_win32.cpp)
    endif()

    if(TARGET SDL2::SDL2main)
        target_link_libraries(nuked-sc55 PRIVATE SDL2::SDL2main)
    endif()

    target_link_libraries(nuked-sc55 PRIVATE SDL2::SDL2 nuked-sc55-backend)
    target_compile_features(nuked-sc55 PRIVATE cxx_std_23)
    target_enable_warnings(nuked-sc55)
    target_enable_conversion_warnings(nuked-sc55)

    if(USE_RTMIDI)
        target_link_libraries(nuked-sc55 PRIVATE RtMidi::rtmidi)
    endif()

    if(WIN32)
        target_link_libraries(nuked-sc55 PRIVATE shlwapi winmm)
    endif()

    if(APPLE)
        find_library(LIBCoreAudio CoreAudio)
        target_link_libraries(nuked-sc55 PRIVATE ${LIBCoreAudio})
    endif()

    if(NUKED_ENABLE_ASIO)
        if (NOT IS_DIRECTORY "${NUKED_ASIO_SDK_DIR}")
            message(FATAL_ERROR "Since NUKED_ENABLE_ASIO is ON, NUKED_ASIO_SDK_DIR"
                " must point to a directory containing the ASIO SDK.")
        endif()

        add_library(asio_sdk)
        target_sources(asio_sdk
            PRIVATE
            "${NUKED_ASIO_SDK_DIR}/common/asio.cpp"
            "${NUKED_ASIO_SDK_DIR}/host/asiodrivers.cpp"
            "${NUKED_ASIO_SDK_DIR}/host/pc/asiolist.cpp"
        )
        target_include_directories(asio_sdk PUBLIC "${NUKED_ASIO_SDK_DIR}/common")
        target_include_directories(asio_sdk PUBLIC "${NUKED_ASIO_SDK_DIR}/host")
        target_include_directories(asio_sdk PUBLIC "${NUKED_ASIO_SDK_DIR}/host/pc")
        target_disable_warnings(asio_sdk)

        target_sources(nuked-sc55
            PRIVATE
            src/standard/output_asio.cpp src/standard/output_asio.h
        )
        target_link_libraries(nuked-sc55 PRIVATE asio_sdk)
    endif()

    install(TARGETS nuked-sc55)
endif()

#==============================================================================
# Renderer Frontend
#==============================================================================
add_executable(nuked-sc55-render)
target_sources(nuked-sc55-render
    PRIVATE
    src/renderer/main.cpp
    src/renderer/smf.cpp
    src/renderer/wav.cpp

    PRIVATE FILE_SET headers TYPE HEADERS FILES
    src/renderer/smf.h
    src/renderer/wav.h
)

target_link_libraries(nuked-sc55-render PRIVATE nuked-sc55-backend)
target_compile_features(nuked-sc55-render PRIVATE cxx_std_23)
target_enable_warnings(nuked-sc55-render)
target_enable_conversion_warnings(nuked-sc55-render)

#==============================================================================
# Installables
#==============================================================================
install(TARGETS nuked-sc55-render)
install(DIRECTORY doc/ DESTINATION ${CMAKE_INSTALL_DOCDIR})
