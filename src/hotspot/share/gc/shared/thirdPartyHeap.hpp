


#ifndef SHARE_GC_SHARED_THIRD_PARTY_HEAP_HPP
#define SHARE_GC_SHARED_THIRD_PARTY_HEAP_HPP



class Thread;
class GCArguments;

namespace third_party_heap {

class MutatorContext {};

extern MutatorContext* bind_mutator(::Thread* current);
extern GCArguments* new_gc_arguments();

};

#endif