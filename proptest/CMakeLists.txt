### library
SET(proptest_sources
    generator/bool.cpp
    generator/integral.cpp
    generator/floating.cpp
    generator/string.cpp
    generator/utf8string.cpp
    generator/utf16string.cpp
    generator/cesu8string.cpp
    generator/util.cpp
    generator/unicode.cpp
    shrinker/bool.cpp
    shrinker/floating.cpp
    shrinker/string.cpp
    shrinker/listlike.cpp
    combinator/intervals.cpp
    util/fork.cpp
    util/utf8string.cpp
    util/utf16string.cpp
    util/cesu8string.cpp
    util/unicode.cpp
    util/printing.cpp
    util/bitmap.cpp
    Stream.cpp
    Shrinkable.cpp
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

#TARGET_PRECOMPILE_HEADERS(proptest
#    PUBLIC
#    proptest.hpp
#    PRIVATE
#)

### tests
SET(proptest_testsources
    test/testbase.cpp
    test/test_gen.cpp
    test/test_property.cpp
    test/test_primitive.cpp
    test/test_combinator.cpp
    test/test_combinator2.cpp
    test/test_shrinkable.cpp
    test/test_mock.cpp
    test/test_util.cpp
    test/test_state_class.cpp
    test/test_state_func.cpp
    test/test_concurrency_class.cpp
    test/test_concurrency_class2.cpp
    test/test_concurrency_func.cpp
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

### compile
SET(compile_sources
    test/compile/unicode.cpp
    test/compile/property.cpp
    test/compile/printing.cpp
    test/compile/just.cpp
    test/compile/lazy.cpp
    test/compile/filter.cpp
    test/compile/derive.cpp
    test/compile/aggregate.cpp
    test/compile/accumulate.cpp
    test/compile/oneof.cpp
    test/compile/elementof.cpp
    test/compile/integral.cpp
    test/compile/floating.cpp
    test/compile/vector.cpp
    test/compile/map.cpp
    test/compile/flatmap.cpp
    test/compile/string.cpp
    test/compile/pair.cpp
    test/compile/tuple.cpp
    test/compile/shrinkable.cpp
    test/compile/stream.cpp
    test/compile/example.cpp
    test/compile/arbitrary.cpp
    test/compile/container.cpp
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
