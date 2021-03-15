#ifndef SHARE_GC_SHARED_THIRD_PARTY_HEAP_HPP
#define SHARE_GC_SHARED_THIRD_PARTY_HEAP_HPP

#include "utilities/macros.hpp"

class Thread;
class GCArguments;

namespace third_party_heap {

extern GCArguments* new_gc_arguments();
extern void register_finalizer(void* obj);

};  // namespace third_party_heap

#include THIRD_PARTY_HEAP_FILE(thirdPartyHeapMutator.hpp)

#endif
