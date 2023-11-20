include_directories(proptest . googletest/googletest/include googletest/googlemock/include/)

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
    PUBLIC
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
