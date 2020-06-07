
SET(CMAKE_VERBOSE_MAKEFILE ON)
CMAKE_MINIMUM_REQUIRED(VERSION 3.0.2)

PROJECT("PropertyBasedTesting"  VERSION 1.0)

SET(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
SET(CMAKE_CXX_FLAGS_RELEASE "")
SET(CMAKE_CXX_STANDARD 14)
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -v -stdlib=libc++ -std=c++14")
#SET(CMAKE_CXX_FLAGS "-Werror -Weverything -Wno-thread-safety-analysis -Wunreachable-code-loop-increment -Wno-error=deprecated -Wno-c++98-compat -Wno-shadow-uncaptured-local -Wno-shadow -Wno-c++98-compat-pedantic")
#SET(CMAKE_CXX_FLAGS "-Werror -Weverything -Wno-thread-safety-analysis -Wunreachable-code-loop-increment -Wno-error=deprecated -Wno-c++98-compat -Wno-shadow-uncaptured-local -Wno-shadow -Wno-c++98-compat-pedantic -Wno-inconsistent-missing-destructor-override -Wno-exit-time-destructors -Wno-sign-conversion -Wno-double-promotion -Wno-implicit-int-conversion -Wno-weak-vtables -Wno-shorten-64-to-32 -Wno-extra-semi -Wno-deprecated -Wno-undef -Wno-undefined-reinterpret-cast -Wno-redundant-parens -Wno-missing-noreturn -Wno-zero-as-null-pointer-constant -Wno-shift-sign-overflow -Wno-newline-eof -Wno-missing-prototypes -Wno-padded -Wno-global-constructors -Wno-unused-parameter -Wno-c++17-extensions -Wno-sign-compare -Wno-covered-switch-default -Wno-format-nonliteral -Wno-used-but-marked-unused -Wno-unused-template -Wno-switch-enum -Wno-unused-member-function -Wno-old-style-cast -Wno-unused-variable -Wno-unreachable-code -Wno-implicit-float-conversion -Wno-gnu-zero-variadic-macro-arguments -Wno-unused-exception-parameter -Wno-unused-local-typedef")
#SET(CMAKE_CXX_LDFLAGS "${CMAKE_CXX_LDFLAGS} -lc++")

add_subdirectory("googletest")

include_directories(. googletest/googletest/include googletest/googlemock/include/)

### library
SET(proptest_sources
    generator/numeric.cpp
    generator/string.cpp
    generator/utf8string.cpp
    generator/utf16string.cpp
    generator/cesu8string.cpp
    generator/util.cpp
    util/fork.cpp
    util/utf8string.cpp
    util/utf16string.cpp
    util/cesu8string.cpp
    util/unicode.cpp
    util/printing.cpp
    Property.cpp
    PropertyContext.cpp
    Random.cpp
    assert.cpp
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
    test/test_state2.cpp
    test/test_concurrency.cpp
    test/test_stream.cpp
    test/test_fork.cpp
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
