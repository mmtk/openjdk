/*
 * Copyright (c) 2001, 2017, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contactSUn 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#ifndef SHARE_VM_GC_MMTK_MMTKHEAP_HPP
#define SHARE_VM_GC_MMTK_MMTKHEAP_HPP

#include "gc/shared/collectedHeap.hpp"
#include "gc/shared/collectorPolicy.hpp"
#include "gc/shared/gcPolicyCounters.hpp"
#include "gc/shared/gcWhen.hpp"
#include "gc/shared/strongRootsScope.hpp"
#include "memory/metaspace.hpp"
#include "utilities/growableArray.hpp"
#include "utilities/ostream.hpp"


class GCMemoryManager;
class MemoryPool;

class MMTkHeap : public CollectedHeap {
    NoPolicy* _collector_policy;

 public:
     
  MMTkHeap(NoPolicy* policy) : CollectedHeap(), _collector_policy(policy) { }
     
  static HeapWord* allocate_from_tlab(Klass* klass, Thread* thread, size_t size);
 
  virtual jint initialize();
  
  virtual HeapWord* mem_allocate(size_t size, bool* gc_overhead_limit_was_exceeded);
  
  
  
  ///Methods implemented in the header-----------------------------------------
  virtual Name kind() const {
    return CollectedHeap::MMTkHeap;
  }
  virtual const char* name() const {
    return "MMTk";
  }
  
  virtual size_t capacity() const { guarantee(false, "capacity not supported"); return 0;}
  virtual size_t used() const { guarantee(false, "used not supported"); return 0;}
  
  virtual bool is_maximal_no_gc() const { guarantee(false, "is_maximal_no_gc  not supported"); return false;}

  virtual size_t max_capacity() const {guarantee(false, "max capacity not supported"); return 0;}
  virtual bool is_in(const void* p) const {guarantee(false, "is in not supported"); return false; }
  
   virtual bool supports_tlab_allocation() const {guarantee(false, "supports-tlab-allocation buffers not supported"); return false;}

  // The amount of space available for thread-local allocation buffers.
  virtual size_t tlab_capacity(Thread *thr) const {guarantee(false, "tlab_capacity not supported");return 0;}

  // The amount of used space for thread-local allocation buffers for the given thread.
  virtual size_t tlab_used(Thread *thr) const { guarantee(false, "tlab_used not supported"); return 0;}
  
  
  
  virtual bool can_elide_tlab_store_barriers() const {guarantee(false, "tcan elide tlab store barriers not supported"); return false;}


  virtual bool can_elide_initializing_store_barrier(oop new_obj) {guarantee(false, "can elide initializing store barrier not supported");return false;}
  
  // mark to be thus strictly sequenced after the stores.
  virtual bool card_mark_must_follow_store() const {guarantee(false, "tcard mark must follow store not supported");return false;}

  virtual void collect(GCCause::Cause cause) {guarantee(false, "collect not supported");}

  // Perform a full collection
  virtual void do_full_collection(bool clear_all_soft_refs) {guarantee(false, "do full collection not supported");}


  // Return the CollectorPolicy for the heap
  virtual CollectorPolicy* collector_policy() const {return _collector_policy;}

  virtual GrowableArray<GCMemoryManager*> memory_managers() {
      guarantee(false, "memory managers not supported");
    GrowableArray<GCMemoryManager*> memory_managers(0);
     return memory_managers;
  
  }
  virtual GrowableArray<MemoryPool*> memory_pools() {
      guarantee(false, "memory pools not supported");
      GrowableArray<MemoryPool*> memory_pools(0);
    return memory_pools;
  }

  // Iterate over all objects, calling "cl.do_object" on each.
  virtual void object_iterate(ObjectClosure* cl) { guarantee(false, "object iterate not supported");}

  // Similar to object_iterate() except iterates only
  // over live objects.
  virtual void safe_object_iterate(ObjectClosure* cl) { guarantee(false, "safe object iterate not supported");}

  virtual HeapWord* block_start(const void* addr) const {guarantee(false, "block start not supported"); return NULL;}

  virtual size_t block_size(const HeapWord* addr) const { guarantee(false, "block size not supported"); return 0; }

  virtual bool block_is_obj(const HeapWord* addr) const { guarantee(false, "block is obj not supported"); return false; }

  virtual jlong millis_since_last_gc() {guarantee(false, "time since last gc not supported"); return 0; }


  virtual void prepare_for_verify() {guarantee(false, "prepare for verify not supported");}


 private:

  virtual void initialize_serviceability() {guarantee(false, "initialize serviceability not supported");}

 public:
  
  // Print heap information on the given outputStream.
  virtual void print_on(outputStream* st) const {guarantee(false, "print on not supported");}


  // Print all GC threads (other than the VM thread)
  // used by this heap.
  virtual void print_gc_threads_on(outputStream* st) const {guarantee(false, "print gc threads on not supported");}

  // Iterator for all GC threads (other than VM thread)
  virtual void gc_threads_do(ThreadClosure* tc) const {guarantee(false, "gc threads do not supported");}

  // Print any relevant tracing info that flags imply.
  // Default implementation does nothing.
  virtual void print_tracing_info() const {guarantee(false, "paint tracing info not supported");}


  // An object is scavengable if its location may move during a scavenge.
  // (A scavenge is a GC which is not a full GC.)
  virtual bool is_scavengable(oop obj) {return false;}
  // Registering and unregistering an nmethod (compiled code) with the heap.
  // Override with specific mechanism for each specialized heap type.

  // Heap verification
  virtual void verify(VerifyOption option) {guarantee(false, "tverify not supported");}

  void post_initialize();

};


#endif // SHARE_VM_GC_MMTK_MMTKHEAP_HPP
