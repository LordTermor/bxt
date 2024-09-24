find_package(Catch2 REQUIRED)

file(GLOB_RECURSE TEST_SOURCES "*.cpp")
file(GLOB_RECURSE BXT_SOURCES "../*/**.cpp")


add_executable(daemon_tests 
    ${TEST_SOURCES} ${BXT_SOURCES}
)

target_link_libraries(daemon_tests PRIVATE 
    Catch2::Catch2WithMain 
    deps 
    reflectcpp 
    Dexode::EventBus
 )

target_include_directories(daemon_tests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../ 
    ${lmdbxx_SOURCE_DIR}/include
)


include(CTest)
include(Catch)

catch_discover_tests(daemon_tests)
