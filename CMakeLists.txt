cmake_minimum_required(VERSION 3.16)
project(QTask VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Core Concurrent)

# find_package(QSignalWaiter REQUIRED)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_TESTS "Build tests" OFF)
option(BUILD_EXAMPLES "Build examples" OFF)
option(BUILD_COROUTINES "Build coroutine support" OFF)

if(BUILD_COROUTINES)
    include(CheckCXXCompilerFlag)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        check_cxx_compiler_flag("/await" COROUTINES_SUPPORTED)
        if(COROUTINES_SUPPORTED)
            set(COROUTINE_FLAG "/await")
        endif()
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        check_cxx_compiler_flag("-fcoroutines" COROUTINES_SUPPORTED)
        if(COROUTINES_SUPPORTED)
            set(COROUTINE_FLAG "-fcoroutines")
        endif()
    endif()
    
    if(NOT COROUTINES_SUPPORTED)
        message(WARNING "C++20 coroutines not supported by compiler")
        set(BUILD_COROUTINES OFF)
    endif()
endif()

add_library(QTask)

target_sources(QTask
    PRIVATE
        src/qtask.cpp
        src/qfunctiontask.cpp
        src/qslottask.cpp
        src/qtasklauncher.cpp
        include/qtask/qtask.h
        include/qtask/qtask_impl.h
        include/qtask/qfunctiontask.h
        include/qtask/qslottask.h
        include/qtask/qtasklauncher.h
        include/qtask/qtask_global.h
        include/qtask/qtask_forward.h
)

target_include_directories(QTask
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(QTask
    PUBLIC
        Qt6::Core
        Qt6::Concurrent
#        QSignalWaiter::QSignalWaiter
)

if(BUILD_COROUTINES)
    target_compile_definitions(QTask
        PUBLIC
            QTASK_HAS_COROUTINES
    )
    target_compile_options(QTask
        PRIVATE
            ${COROUTINE_FLAG}
    )
endif()

set_target_properties(QTask PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Generate export header
include(GenerateExportHeader)
generate_export_header(QTask
    BASE_NAME QTASK
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/qtask_export.h
)

include(GNUInstallDirs)

install(TARGETS QTask
    EXPORT QTaskTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES
    include/qtask/qtask.h
    include/qtask/qtask_impl.h
    include/qtask/qfunctiontask.h
    include/qtask/qslottask.h
    include/qtask/qtasklauncher.h
    include/qtask/qtask_global.h
    include/qtask/qtask_forward.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtask
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/qtask_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtask
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/QTaskConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/QTaskConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QTask
    PATH_VARS
        CMAKE_INSTALL_INCLUDEDIR
        CMAKE_INSTALL_LIBDIR
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/QTaskConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/QTaskConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/QTaskConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QTask
)

install(EXPORT QTaskTargets
    NAMESPACE QTask::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QTask
)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
