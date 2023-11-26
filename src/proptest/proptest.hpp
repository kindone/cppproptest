#pragma once

#include "proptest/api.hpp"
#include "proptest/util/std.hpp"
#include "proptest/util/any.hpp"
#include "proptest/Stream.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/GenBase.hpp"
#include "proptest/assert.hpp"
#include "proptest/Random.hpp"
#include "proptest/gen.hpp"
#include "proptest/util/action.hpp"
#include "proptest/util/typelist.hpp"
#include "proptest/util/function_traits.hpp"
#include "proptest/util/createGenTuple.hpp"
#include "proptest/util/invokeWithArgs.hpp"
#include "proptest/util/invokeWithGenTuple.hpp"
#include "proptest/util/bitmap.hpp"
#include "proptest/util/nullable.hpp"
#include "proptest/util/tuple.hpp"
#include "proptest/util/unicode.hpp"
#include "proptest/util/utf8string.hpp"
#include "proptest/util/utf16string.hpp"
#include "proptest/util/cesu8string.hpp"
#include "proptest/util/fork.hpp"
#include "proptest/util/printing.hpp"
#include "proptest/util/misc.hpp"
#include "proptest/shrinkers.hpp"
#include "proptest/generators.hpp"
#include "proptest/PropertyContext.hpp"
#include "proptest/PropertyBase.hpp"
#include "proptest/Property.hpp"

/**
 * @file proptest.hpp
 * @brief Main header file for your `cpppropetest` property-based tests
 */

/**
 * @defgroup Combinators Generator Combinators
 * @defgroup Generators Generators
 * @defgroup Property Property
 */
