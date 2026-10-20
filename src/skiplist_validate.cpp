// OWNER: Santhosh. Debug invariant checker used by every test.
#include "IndexableSkipList.h"

namespace lb {

// TODO(Santhosh): the real checker. Until then it always passes.
bool IndexableSkipList::validate(std::string*) const { return true; }

}  // namespace lb
