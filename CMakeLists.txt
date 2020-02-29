
set(CMAKE_VERBOSE_MAKEFILE ON)
cmake_minimum_required(VERSION 3.0.2)

project("PropertyBasedTesting"  VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory("googletest")

include_directories(. googletest/googletest/include googletest/googlemock/include/)

### library
SET(proptest_sources
    testing/generator/numeric.cpp
    testing/generator/string.cpp
    testing/property.cpp
    testing/Random.cpp
)

ADD_LIBRARY(proptest
    SHARED
    ${proptest_sources}
    #NO_PACKAGE
)

set_target_properties(proptest PROPERTIES
	COMPILE_FLAGS "-DPROPTEST_DLL -DPROTEST_DLL_EXPORTS")

TARGET_LINK_LIBRARIES(proptest
    PRIVATE
)


### tests
SET(proptest_testsources
    test_gen.cpp
    test_mock.cpp
    test_util.cpp
)


ADD_EXECUTABLE(test_proptest
    ${proptest_testsources}
)

TARGET_LINK_LIBRARIES(test_proptest
    PRIVATE
        proptest
	gtest_main
	gmock_main
)

ADD_TEST(NAME proptest_gtest COMMAND test_proptest)
