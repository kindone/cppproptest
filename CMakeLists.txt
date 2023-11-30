
SET(CMAKE_VERBOSE_MAKEFILE ON)
#CMAKE_MINIMUM_REQUIRED(VERSION 3.16)
CMAKE_MINIMUM_REQUIRED(VERSION 3.0.2)

PROJECT("cppproptest"  VERSION 1.0)

#SET(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
SET(CMAKE_CXX_FLAGS_RELEASE "")
SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")
SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wimplicit-int-conversion -Werror=unused-local-typedefs -Wsign-compare")
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -v -stdlib=libc++ -std=c++14")
#SET(CMAKE_CXX_FLAGS "-Werror -Weverything -Wno-thread-safety-analysis -Wunreachable-code-loop-increment -Wno-error=deprecated -Wno-c++98-compat -Wno-shadow-uncaptured-local -Wno-shadow -Wno-c++98-compat-pedantic")
#SET(CMAKE_CXX_FLAGS "-Werror -Weverything -Wno-thread-safety-analysis -Wunreachable-code-loop-increment -Wno-error=deprecated -Wno-c++98-compat -Wno-shadow-uncaptured-local -Wno-shadow -Wno-c++98-compat-pedantic -Wno-inconsistent-missing-destructor-override -Wno-exit-time-destructors -Wno-sign-conversion -Wno-double-promotion -Wno-implicit-int-conversion -Wno-weak-vtables -Wno-shorten-64-to-32 -Wno-extra-semi -Wno-deprecated -Wno-undef -Wno-undefined-reinterpret-cast -Wno-redundant-parens -Wno-missing-noreturn -Wno-zero-as-null-pointer-constant -Wno-shift-sign-overflow -Wno-newline-eof -Wno-missing-prototypes -Wno-padded -Wno-global-constructors -Wno-unused-parameter -Wno-c++17-extensions -Wno-sign-compare -Wno-covered-switch-default -Wno-format-nonliteral -Wno-used-but-marked-unused -Wno-unused-template -Wno-switch-enum -Wno-unused-member-function -Wno-old-style-cast -Wno-unused-variable -Wno-unreachable-code -Wno-implicit-float-conversion -Wno-gnu-zero-variadic-macro-arguments -Wno-unused-exception-parameter -Wno-unused-local-typedef")
#SET(CMAKE_CXX_LDFLAGS "${CMAKE_CXX_LDFLAGS} -lc++")

#SET(CMAKE_CXX_FLAGS "-Werror -Wignored-qualifiers -Wimplicit-fallthrough -Wunused-variable -Wcast-qual -Wunused-local-typedefs -Wunused-function -Wmissing-field-initializers -Wunused-parameter -Warray-bounds -Wextra-semi -Wshadow")

add_subdirectory("proptest")
add_subdirectory("googletest")

include_directories(. googletest/googletest/include googletest/googlemock/include/)

### library
SET(proptest_sources
    proptest/generator/bool.cpp
    proptest/generator/integral.cpp
    proptest/generator/floating.cpp
    proptest/generator/string.cpp
    proptest/generator/utf8string.cpp
    proptest/generator/utf16string.cpp
    proptest/generator/cesu8string.cpp
    proptest/generator/util.cpp
    proptest/generator/unicode.cpp
    proptest/shrinker/bool.cpp
    proptest/shrinker/floating.cpp
    proptest/shrinker/string.cpp
    proptest/shrinker/listlike.cpp
    proptest/combinator/intervals.cpp
    proptest/util/fork.cpp
    proptest/util/utf8string.cpp
    proptest/util/utf16string.cpp
    proptest/util/cesu8string.cpp
    proptest/util/unicode.cpp
    proptest/util/printing.cpp
    proptest/util/bitmap.cpp
    proptest/Stream.cpp
    proptest/Shrinkable.cpp
    proptest/Property.cpp
    proptest/PropertyContext.cpp
    proptest/Random.cpp
    proptest/assert.cpp
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

#TARGET_PRECOMPILE_HEADERS(proptest
#    PUBLIC
#    proptest.hpp
#    PRIVATE
#)

### tests
SET(proptest_testsources
    proptest/test/testbase.cpp
    proptest/test/test_gen.cpp
    proptest/test/test_property.cpp
    proptest/test/test_primitive.cpp
    proptest/test/test_combinator.cpp
    proptest/test/test_combinator2.cpp
    proptest/test/test_shrinkable.cpp
    proptest/test/test_mock.cpp
    proptest/test/test_util.cpp
    proptest/test/test_state_class.cpp
    proptest/test/test_state_func.cpp
    proptest/test/test_concurrency_class.cpp
    proptest/test/test_concurrency_class2.cpp
    proptest/test/test_concurrency_func.cpp
    proptest/test/test_stream.cpp
    proptest/test/test_fork.cpp
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

### compile
SET(compile_sources
    proptest/test/compile/unicode.cpp
    proptest/test/compile/property.cpp
    proptest/test/compile/printing.cpp
    proptest/test/compile/just.cpp
    proptest/test/compile/lazy.cpp
    proptest/test/compile/filter.cpp
    proptest/test/compile/derive.cpp
    proptest/test/compile/aggregate.cpp
    proptest/test/compile/accumulate.cpp
    proptest/test/compile/oneof.cpp
    proptest/test/compile/elementof.cpp
    proptest/test/compile/integral.cpp
    proptest/test/compile/floating.cpp
    proptest/test/compile/vector.cpp
    proptest/test/compile/map.cpp
    proptest/test/compile/flatmap.cpp
    proptest/test/compile/string.cpp
    proptest/test/compile/pair.cpp
    proptest/test/compile/tuple.cpp
    proptest/test/compile/shrinkable.cpp
    proptest/test/compile/stream.cpp
    proptest/test/compile/example.cpp
    proptest/test/compile/arbitrary.cpp
    proptest/test/compile/container.cpp
)

ADD_EXECUTABLE(compile_proptest
    EXCLUDE_FROM_ALL
    ${compile_sources}
)

TARGET_LINK_LIBRARIES(compile_proptest
    PRIVATE
        proptest
	gtest_main
)

ADD_TEST(compile_proptest_gtest
    EXCLUDE_FROM_ALL
    compile_proptest)
