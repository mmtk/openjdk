#ifndef SHARE_VM_GC_THIRD_PARTY_HEAP_BARRIER_HPP
#define SHARE_VM_GC_THIRD_PARTY_HEAP_BARRIER_HPP

#include "gc/shared/barrierSet.hpp"
#include "gc/shared/barrierSetConfig.hpp"
#include "memory/memRegion.hpp"
#include "oops/access.hpp"
#include "oops/accessBackend.hpp"
#include "oops/oopsHierarchy.hpp"
#include "utilities/fakeRttiSupport.hpp"

// This class provides the interface between a barrier implementation and
// the rest of the system.

class NoBarrier : public BarrierSet {
  friend class VMStructs;
  template <DecoratorSet decorators, typename BarrierSetT = NoBarrier>
  class AccessBarrier: public BarrierSet::AccessBarrier<decorators, BarrierSetT> {};
};


template<>
struct BarrierSet::GetName<NoBarrier> {
  static const BarrierSet::Name value = BarrierSet::NoBarrier;
};

template<>
struct BarrierSet::GetType<BarrierSet::NoBarrier> {
  typedef ::NoBarrier type;
};


#endif