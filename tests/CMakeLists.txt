find_package(GTest REQUIRED)

set(TEST_SOURCES
    test_qtask.cpp
    test_qfunctiontask.cpp
    test_qslottask.cpp
    test_qtasklauncher.cpp
)

add_executable(QTaskTests ${TEST_SOURCES})

target_link_libraries(QTaskTests
    PRIVATE
        QTask
        GTest::gtest
        GTest::gtest_main
        Qt6::Core
)

target_include_directories(QTaskTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
        ${CMAKE_CURRENT_BINARY_DIR}
)

# Включаем AUTOMOC
set_target_properties(QTaskTests PROPERTIES
    AUTOMOC ON
    AUTOUIC OFF
    AUTORCC OFF
)

include(GoogleTest)
gtest_discover_tests(QTaskTests)