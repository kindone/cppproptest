#include "api.hpp"
#include "Shrinkable.hpp"
#include "util/utf8string.hpp"
#include "util/cesu8string.hpp"

namespace proptest {

// explicit instantiation
template class Shrinkable<vector<Shrinkable<util::any>>>;

}  // namespace proptest
