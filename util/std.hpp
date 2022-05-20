#pragma once

#include <list>
#include <vector>
#include <set>
#include <map>
#include <tuple>
#include <type_traits>
#include <initializer_list>
#include <functional>
#include <utility>
#include <memory>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <algorithm>
#include <random>
#include <limits>

#include <chrono>

namespace proptest {

using std::list;
using std::map;
using std::pair;
using std::set;
using std::vector;

using std::tuple;
using std::tuple_element;
using std::tuple_element_t;
using std::tuple_size;

using std::declval;
using std::index_sequence;
using std::initializer_list;
using std::is_trivial;
using std::remove_reference;
using std::remove_reference_t;

namespace util {
using std::back_inserter;
using std::forward;
using std::make_pair;
using std::make_shared;
using std::make_tuple;
using std::make_unique;
using std::move;
using std::transform;
}  // namespace util

using std::get;
using std::is_lvalue_reference;
using std::is_pointer;
using std::is_same;
using std::is_same_v;
using std::make_index_sequence;

using std::decay_t;
using std::enable_if;
using std::enable_if_t;
using std::invoke_result;
using std::invoke_result_t;

using std::shared_ptr;
using std::static_pointer_cast;
using std::to_string;
using std::unique_ptr;

using std::numeric_limits;

using std::function;

using std::cerr;
using std::cout;
using std::endl;
using std::hex;
using std::ios;
using std::ostream;
using std::setfill;
using std::setw;
using std::string;
using std::stringstream;

using std::error_code;
using std::exception;
using std::invalid_argument;
using std::logic_error;
using std::runtime_error;

using std::boolalpha;
using std::mt19937_64;
using std::uniform_int_distribution;
using std::uniform_real_distribution;

using std::false_type;
using std::true_type;

using std::conjunction_v;
using std::is_convertible_v;

}  // namespace proptest
