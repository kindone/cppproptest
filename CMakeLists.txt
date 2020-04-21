
SET(CMAKE_VERBOSE_MAKEFILE ON)
CMAKE_MINIMUM_REQUIRED(VERSION 3.0.2)

PROJECT("PropertyBasedTesting"  VERSION 1.0)

SET(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
SET(CMAKE_CXX_FLAGS_RELEASE "")
SET(CMAKE_CXX_STANDARD 14)
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -v -stdlib=libc++ -std=c++14")
#SET(CMAKE_CXX_LDFLAGS "${CMAKE_CXX_LDFLAGS} -lc++")

add_subdirectory("googletest")

include_directories(. googletest/googletest/include googletest/googlemock/include/)

### library
SET(proptest_sources
    generator/numeric.cpp
    generator/string.cpp
    generator/util.cpp
    Property.cpp
    Random.cpp
    printing.cpp
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
    test/testbase.cpp
    test/test_gen.cpp
    test/test_property.cpp
    test/test_primitive.cpp
    test/test_combinator.cpp
    test/test_shrinkable.cpp
    test/test_mock.cpp
    test/test_util.cpp
    test/test_state.cpp
    test/test_stream.cpp
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
