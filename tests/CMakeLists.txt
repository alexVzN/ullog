include(FetchContent)

find_package(GTest QUIET)
if(NOT GTest_FOUND)
    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

add_executable(ullog_tests
        test_logger.cpp
)

target_link_libraries(ullog_tests PRIVATE
        ullog::ullog
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(ullog_tests)