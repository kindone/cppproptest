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


### compile separate
SET(compile_misc_sources
    proptest/test/compile/unicode.cpp
    proptest/test/compile/container.cpp
)

ADD_EXECUTABLE(compile_misc_proptest
    EXCLUDE_FROM_ALL
    ${compile_misc_sources}
)

TARGET_LINK_LIBRARIES(compile_misc_proptest
    PRIVATE
        proptest
	gtest_main
)

SET(compile_shrinkable_sources
    proptest/test/compile/shrinkable.cpp
    proptest/test/compile/stream.cpp
)

ADD_EXECUTABLE(compile_shrinkable_proptest
    EXCLUDE_FROM_ALL
    ${compile_shrinkable_sources}
)

TARGET_LINK_LIBRARIES(compile_shrinkable_proptest
    PRIVATE
        proptest
	gtest_main
)

SET(compile_generator_sources
    proptest/test/compile/integral.cpp
    proptest/test/compile/floating.cpp
    proptest/test/compile/vector.cpp
    proptest/test/compile/string.cpp
    proptest/test/compile/map.cpp
)

ADD_EXECUTABLE(compile_generator_proptest
    EXCLUDE_FROM_ALL
    ${compile_generator_sources}
)

TARGET_LINK_LIBRARIES(compile_generator_proptest
    PRIVATE
        proptest
	gtest_main
)

SET(compile_combinator_sources
    proptest/test/compile/just.cpp
    proptest/test/compile/lazy.cpp
    proptest/test/compile/filter.cpp
    proptest/test/compile/derive.cpp
    proptest/test/compile/aggregate.cpp
    proptest/test/compile/accumulate.cpp
    proptest/test/compile/oneof.cpp
    proptest/test/compile/elementof.cpp
)

ADD_EXECUTABLE(compile_combinator_proptest
    EXCLUDE_FROM_ALL
    ${compile_combinator_sources}
)

TARGET_LINK_LIBRARIES(compile_combinator_proptest
    PRIVATE
        proptest
	gtest_main
)


SET(compile_just_sources
    proptest/test/compile/just.cpp
)

ADD_EXECUTABLE(compile_just_proptest
    EXCLUDE_FROM_ALL
    ${compile_just_sources}
)

TARGET_LINK_LIBRARIES(compile_just_proptest
    PRIVATE
        proptest
	gtest_main
)


SET(compile_container_sources
    proptest/test/compile/container.cpp
)

ADD_EXECUTABLE(compile_container_proptest
    EXCLUDE_FROM_ALL
    ${compile_container_sources}
)

TARGET_LINK_LIBRARIES(compile_container_proptest
    PRIVATE
        proptest
	gtest_main
)


SET(compile_property_sources
    proptest/test/compile/property.cpp
)

ADD_EXECUTABLE(compile_property_proptest
    EXCLUDE_FROM_ALL
    ${compile_property_sources}
)

TARGET_LINK_LIBRARIES(compile_property_proptest
    PRIVATE
        proptest
	gtest_main
)


SET(compile_example_sources
    proptest/test/compile/example.cpp
)

ADD_EXECUTABLE(compile_example_proptest
    EXCLUDE_FROM_ALL
    ${compile_example_sources}
)

TARGET_LINK_LIBRARIES(compile_example_proptest
    PRIVATE
        proptest
	gtest_main
)

ADD_TEST(compile_misc_gtest
    EXCLUDE_FROM_ALL
    compile_misc_proptest)

ADD_TEST(compile_shrinkable_gtest
    EXCLUDE_FROM_ALL
    compile_shrinkable_proptest)

ADD_TEST(compile_generator_gtest
    EXCLUDE_FROM_ALL
    compile_generator_proptest)

ADD_TEST(compile_combinator_gtest
    EXCLUDE_FROM_ALL
    compile_combinator_proptest)

ADD_TEST(compile_just_gtest
    EXCLUDE_FROM_ALL
    compile_property_proptest)
ADD_TEST(compile_property_gtest
    EXCLUDE_FROM_ALL
    compile_property_proptest)


ADD_CUSTOM_TARGET(compile_part_proptest
DEPENDS
compile_property_proptest
compile_just_proptest
compile_generator_proptest
compile_combinator_proptest
compile_shrinkable_proptest
compile_misc_proptest
compile_example_proptest
compile_container_proptest
)
