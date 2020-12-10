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

using std::vector;
using std::list;
using std::set;
using std::map;
using std::pair;

using std::tuple;
using std::tuple_size;
using std::tuple_element;
using std::tuple_element_t;

using std::index_sequence;
using std::initializer_list;
using std::remove_reference;
using std::remove_reference_t;
using std::is_trivial;
using std::declval;

namespace util {
using std::forward;
using std::move;
using std::make_pair;
using std::make_tuple;
using std::make_shared;
using std::transform;
using std::back_inserter;
}

using std::make_index_sequence;
using std::get;
using std::is_lvalue_reference;
using std::is_pointer;
using std::is_same;

using std::decay_t;
using std::result_of;
using std::enable_if;
using std::enable_if_t;

using std::shared_ptr;
using std::to_string;

using std::numeric_limits;

using std::function;

using std::string;
using std::ostream;
using std::stringstream;
using std::cerr;
using std::cout;
using std::endl;
using std::ios;
using std::setw;
using std::setfill;
using std::hex;

using std::exception;
using std::logic_error;
using std::runtime_error;
using std::invalid_argument;
using std::error_code;

using std::uniform_int_distribution;
using std::boolalpha;
using std::mt19937_64;

using std::true_type;
using std::false_type;

} // namespace proptest
