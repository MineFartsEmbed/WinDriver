# Jungo Connectivity Confidential. Copyright (c) 2026 Jungo Connectivity Ltd.
# https://www.jungo.com

if ("${USER_BITS}" STREQUAL "" OR "${PLATFORM}" STREQUAL "")
    if(NOT WIN32 AND NOT APPLE)
        execute_process(
            COMMAND uname -m
            COMMAND tr -d '\n'
            OUTPUT_VARIABLE UNAMEM
            )

        if ("${UNAMEM}" STREQUAL "x86")
            set(USER_BITS 32)
            set(PLATFORM "x86")
        elseif ("${UNAMEM}" STREQUAL "armv7l")
            set(USER_BITS 32)
            set(PLATFORM "ARM")
        elseif ("${UNAMEM}" STREQUAL "arm64" OR
                "${UNAMEM}" STREQUAL "aarch64")
            set(USER_BITS 64)
            set(PLATFORM "ARM64")
        else() # x86_64 is the default
            set(USER_BITS 64)
            set(PLATFORM "x86_64")
        endif()
    elseif(WIN32)
        execute_process(
            COMMAND powershell 
            "(wmic os get osarchitecture | Select-String -Pattern '\\d+|ARM' -AllMatches).Matches.Value.Trim()"
            OUTPUT_VARIABLE WMICOS
            )
        if ("${WMICOS}" MATCHES "32")
            set(USER_BITS 32)
            set(PLATFORM "x86")
        elseif ("${WMICOS}" MATCHES "ARM")
            set(USER_BITS 64)
            set(PLATFORM "ARM64")
        else() # x86_64 is the default
            set(USER_BITS 64)
            set(PLATFORM "x86_64")
        endif()
    endif()
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release")
endif()

file(RELATIVE_PATH PROJECT_DIR ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

# VERSION DEFINES

file(STRINGS ${CMAKE_CURRENT_LIST_DIR}/wd_ver.h wd_ver)
string(REGEX MATCH "WD_MAJOR_VER ([0-9]*)" _ ${wd_ver})
set(WD_MAJOR_VER ${CMAKE_MATCH_1})
string(REGEX MATCH "WD_MINOR_VER ([0-9]*)" _ ${wd_ver})
set(WD_MINOR_VER ${CMAKE_MATCH_1})
string(REGEX MATCH "WD_SUB_MINOR_VER ([0-9]*)" _ ${wd_ver})
set(WD_SUB_MINOR_VER ${CMAKE_MATCH_1})
string(CONCAT WD_VERSION "${WD_MAJOR_VER}${WD_MINOR_VER}${WD_SUB_MINOR_VER}")
string(CONCAT WD_VERSION_DOTS "${WD_MAJOR_VER}.${WD_MINOR_VER}.${WD_SUB_MINOR_VER}")

file(STRINGS ${CMAKE_CURRENT_LIST_DIR}/wd_ver.proj wd_ver_proj)
string(REGEX MATCH "<NetcoreVersion>(.*)<\\/NetcoreVersion>" _ ${wd_ver_proj})
set(NetcoreVersion ${CMAKE_MATCH_1})

# Sets WD_BASEDIR to the parent folder of this file. DO NOT change this
set(WD_BASEDIR ${CMAKE_CURRENT_LIST_DIR}/..)
# Populate driver name
set(DRIVER_NAME windrvr${WD_VERSION})
set(SERVICE_NAME WinDriver${WD_VERSION})
set(WDHWID *WINDRVR${WD_VERSION})

# SHARED DEPENDENCIES
find_package(Threads)

set (SAMPLE_SHARED_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/../samples/c/shared/wdc_diag_lib.c
    ${CMAKE_CURRENT_LIST_DIR}/../samples/c/shared/diag_lib.c
    ${CMAKE_CURRENT_LIST_DIR}/../samples/c/shared/pci_menus_common.c
    ${CMAKE_CURRENT_LIST_DIR}/../samples/c/shared/diag_log_lib.c
    ${CMAKE_CURRENT_LIST_DIR}/../samples/c/shared/wdc_diag_platform_lib.c)

# ENVIRONMENT SPECIFIC DEFINES
# 64 application or 32 on 64 application
if ("${USER_BITS}" STREQUAL "64" OR DEFINED USER_32_ON_64)
    set(PLAT_DIR "amd64")
else()
    set(PLAT_DIR "x86")
endif()
message (STATUS "Compiling for ${USER_BITS}-bit (${PROJECT_DIR})")

if (UNIX)
    add_definitions("-DUNIX")

    add_definitions(-O2)
    add_definitions("-Wno-unused-result -Wno-write-strings ")
    if (NOT APPLE)
        message (STATUS "Compiling for LINUX")
        set(ARCH LINUX)
        set(OS LN)
        if ("${PLATFORM}" STREQUAL "ARM")
            set(KERNEL_BITS 32)
        elseif("${PLATFORM}" STREQUAL "ARM64")
            set(KERNEL_BITS 64)
            set(RUNTIME_ID "linux-arm64")
        elseif("${PLATFORM}" STREQUAL "x86")
            set(KERNEL_BITS 32)
        elseif("${PLATFORM}" STREQUAL "x86_64")
            set(KERNEL_BITS 64)
            set(RUNTIME_ID "linux-x64")
        else()
            message(FATAL_ERROR
                "WinDriver Error: PLATFORM must be set to either x86, x86_64, ARM or ARM64")
        endif()

        set(KERNEL_FLAGS "-D__KERNEL__ -DMODULE -x c -nostdinc
        -Wall -Werror -Wcast-align -Wstrict-prototypes -Wno-trigraphs
        -Wno-sign-compare
        -iwithprefix include -pipe
        -fno-strict-aliasing -fno-common -fno-omit-frame-pointer
        -fno-reorder-blocks -fno-asynchronous-unwind-tables
        -fno-pie -fno-stack-protector")

        if ("${PLATFORM}" STREQUAL "x86_64")
            set(KERNEL_FLAGS "${KERNEL_FLAGS} -march=k8 -mno-red-zone
                -mcmodel=kernel -funit-at-a-time")
        elseif("${PLATFORM}" STREQUAL "x86")
            set(KERNEL_FLAGS "${KERNEL_FLAGS} -mpreferred-stack-boundary=2
                -mregparm=3")
        endif()

        if (NOT "${PLATFORM}" STREQUAL "x86_64")
            set(ARCH_LFLAG -fno-pie)
        endif()

        set (CMAKE_SHARED_LINKER_FLAGS "${ARCH_LFLAG}")

        if (DEFINED USER_32_ON_64)
            set(WDAPI_LIB "wdapi${WD_VERSION}_32")
            add_compile_options("-m32")
            add_link_options("-m32")
        else()
            set(WDAPI_LIB "wdapi${WD_VERSION}")
        endif()
        
    elseif (APPLE)
        message("Compiling for APPLE (${CMAKE_HOST_SYSTEM_PROCESSOR})")
        add_definitions("-DAPPLE")
        set(ARCH APPLE)
        set(USER_BITS 64)
        set(KERNEL_BITS 64)
        set(OS "MAC")

        if ("${CMAKE_HOST_SYSTEM_PROCESSOR}" STREQUAL "arm64")
            set(CMAKE_OSX_ARCHITECTURES "arm64"
                CACHE STRING "Build architectures for Mac OS X" FORCE)
            include_directories(/System/Volumes/Data/Library/Developer/CommandLineTools/SDKs/MacOSX13.1.sdk/System/Library/Frameworks/DriverKit.framework/Versions/A/Headers/)
            set(PLATFORM "ARM64")
            add_definitions("-DTARGET_OS_OSX")
            set(RUNTIME_ID "osx-arm64")
        else()
            set(PLATFORM "x86_64")
            set(CMAKE_OSX_SYSROOT /Users/Shared/MacOSX10.14.sdk/)
            include_directories(/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.14.sdk/System/Library/Frameworks/IOKit.framework/Headers)
            set(RUNTIME_ID "osx-x64")
        endif()

        find_library(WDAPI_LIB wdapi${WD_VERSION} HINTS /usr/local/lib)
        if (${WDAPI_LIB} MATCHES -NOTFOUND)
            set(WDAPI_LIB "wdapi${WD_VERSION}")
        endif()
        set(WDAPI_LIB_NAME "wdapi${WD_VERSION}.dylib")
    endif()

    # CLEANUP
    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${ARCH}/*")
elseif (WIN32)
    message (STATUS "Compiling for WINDOWS")

    if ("${CMAKE_GENERATOR}" STREQUAL "MinGW Makefiles")
        set(PLATFORM "x86_64")
        message ("Compiling with MinGW")
    elseif ("${CMAKE_GENERATOR}" MATCHES "Ninja")
        message ("Compiling with Ninja")
        include_directories("C:/Program Files (x86)/Windows Kits/10/Include/10.0.17763.0/um")
        include_directories("C:/Program Files (x86)/Windows Kits/10/Include/10.0.17763.0/shared")
    else()
        add_definitions(/W3 /wd26498 /wd26812 /MP)
    endif()

    add_definitions(-D_CRT_SECURE_NO_WARNINGS -DWINNT)
    if ("${PLATFORM}" STREQUAL "x86_64" OR DEFINED USER_32_ON_64)
        set(KERNEL_BITS 64)
        set(PLAT_DIR "amd64")
        set(RUNTIME_ID "win-x64")
    elseif("${PLATFORM}" STREQUAL "ARM64")
        set(KERNEL_BITS 64)
        set(PLAT_DIR "arm64")
        set(RUNTIME_ID "win-arm64")
    else()
        set(KERNEL_BITS 32)
        set(PLAT_DIR "x86")
    endif()

    set(OS "")
    set(ARCH WIN32)

    if ("${PLATFORM}" STREQUAL "ARM64")
        set(WDAPI_LIB
        "${WD_BASEDIR}/lib/${PLAT_DIR}/wdapi${WD_VERSION}_arm64.lib")
        set(WDAPI_LIB_NAME "wdapi${WD_VERSION}_arm64.dll")
    elseif ("${USER_BITS}" STREQUAL "32" AND "${KERNEL_BITS}" STREQUAL "64")
        set(WDAPI_LIB
            "${WD_BASEDIR}/lib/${PLAT_DIR}/x86/wdapi${WD_VERSION}_32.lib")
        set(WDAPI_LIB_NAME "wdapi${WD_VERSION}_32.dll")
   else() # Native x86 or x64 compilation
        set(WDAPI_LIB "${WD_BASEDIR}/lib/${PLAT_DIR}/wdapi${WD_VERSION}.lib")
        set(WDAPI_LIB_NAME "wdapi${WD_VERSION}.dll")
    endif()

    if("${PLATFORM}" STREQUAL "x86_64")
        find_library(DIFXAPI difxapi PATHS "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.17763.0/um/x64/")
        set(WIN_PLATFORM_DIR "WINNT.x86_64")
        set(MSBUILD_PLATFORM "x64")
    elseif ("${PLATFORM}" STREQUAL "x86")
        find_library(DIFXAPI difxapi PATHS "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.17763.0/um/x86/")
        set(WIN_PLATFORM_DIR "WINNT.i386")
        set(MSBUILD_PLATFORM "Win32")
    elseif ("${PLATFORM}" STREQUAL "ARM64")
        set(WIN_PLATFORM_DIR "WINNT.ARM64")
        set(MSBUILD_PLATFORM "ARM64")
    endif()
endif()

message(STATUS "${KERNEL_BITS} Bit Kernel Detected")
if(DEFINED USER_32_ON_64)
    message(STATUS "32 bit compilation on 64 Bit Kernel")
    add_definitions(-Dx86_64 -Di386)
elseif ("${PLATFORM}" STREQUAL "ARM")

elseif("${PLATFORM}" STREQUAL "ARM64")

elseif("${PLATFORM}" STREQUAL "x86_64")

else()
    add_definitions(-Dx86 -Di386)
    # 32 on 64
endif()

message("${PLATFORM} compilation")
add_definitions(-D${ARCH} -D${PLATFORM} -DWD_DRIVER_NAME_CHANGE)
