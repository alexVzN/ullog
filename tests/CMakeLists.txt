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

# Each TU below installs its own ULLOG_CREATE_DEFAULT at a distinct threshold,
# so they must live in separate executables.

function(ullog_add_test NAME)
    add_executable(${NAME} ${NAME}.cpp)
    target_link_libraries(${NAME} PRIVATE ullog::ullog GTest::gtest_main)
    gtest_discover_tests(${NAME})
endfunction()

include(GoogleTest)

ullog_add_test(test_logger)         # MAX_SEV = All — open threshold
ullog_add_test(test_filter_error)   # MAX_SEV = Error — Debug/Info/Warn gated
ullog_add_test(test_filter_none)    # MAX_SEV = None — everything gated
ullog_add_test(test_filter_sink)    # runtime per-sink accepted mask
ullog_add_test(test_multisink)      # MultiSink fan-out routing
