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
#include "gc/mmtk/mmtkMemoryPool.hpp"


class GCMemoryManager;
class MemoryPool;
//class mmtkGCTaskManager;

class MMTkHeap : public CollectedHeap {
    NoPolicy* _collector_policy;
    MMTkMemoryPool* _mmtk_pool;
    GCMemoryManager* _mmtk_manager;
    HeapWord* _start;
    HeapWord* _end;
    
private:
   // static mmtkGCTaskManager* _mmtk_gc_task_manager;
    

 public:
     
  MMTkHeap(NoPolicy* policy) : CollectedHeap(), _collector_policy(policy) { }
     
  static HeapWord* allocate_from_tlab(Klass* klass, Thread* thread, size_t size);
 
  virtual jint initialize();
  
  virtual HeapWord* mem_allocate(size_t size, bool* gc_overhead_limit_was_exceeded);
  
  
  
  virtual Name kind() const {   
    return CollectedHeap::MMTkHeap;
  }
  virtual const char* name() const {
    return "MMTk";
  }
  
  virtual size_t capacity() const; 
  virtual size_t used() const;
  
  virtual bool is_maximal_no_gc() const; 

  virtual size_t max_capacity() const; 
  virtual bool is_in(const void* p) const; 
  
   virtual bool supports_tlab_allocation() const; 

  // The amount of space available for thread-local allocation buffers.
  virtual size_t tlab_capacity(Thread *thr) const; 

  // The amount of used space for thread-local allocation buffers for the given thread.
  virtual size_t tlab_used(Thread *thr) const; 
  
  
  
  virtual bool can_elide_tlab_store_barriers() const; 


  virtual bool can_elide_initializing_store_barrier(oop new_obj); 
  
  // mark to be thus strictly sequenced after the stores.
  virtual bool card_mark_must_follow_store() const; 

  virtual void collect(GCCause::Cause cause); 

  // Perform a full collection
  virtual void do_full_collection(bool clear_all_soft_refs); 


  // Return the CollectorPolicy for the heap
  virtual CollectorPolicy* collector_policy() const ;

  virtual GrowableArray<GCMemoryManager*> memory_managers() ;
  virtual GrowableArray<MemoryPool*> memory_pools(); 

  // Iterate over all objects, calling "cl.do_object" on each.
  virtual void object_iterate(ObjectClosure* cl); 

  // Similar to object_iterate() except iterates only
  // over live objects.
  virtual void safe_object_iterate(ObjectClosure* cl) ;

  virtual HeapWord* block_start(const void* addr) const ;

  virtual size_t block_size(const HeapWord* addr) const ;

  virtual bool block_is_obj(const HeapWord* addr) const; 

  virtual jlong millis_since_last_gc() ;


  virtual void prepare_for_verify() ;


 private:

  virtual void initialize_serviceability() ;

 public:
  
  // Print heap information on the given outputStream.
  virtual void print_on(outputStream* st) const ;


  // Print all GC threads (other than the VM thread)
  // used by this heap.
  virtual void print_gc_threads_on(outputStream* st) const; 

  // Iterator for all GC threads (other than VM thread)
  virtual void gc_threads_do(ThreadClosure* tc) const; 

  // Print any relevant tracing info that flags imply.
  // Default implementation does nothing.
  virtual void print_tracing_info() const ;


  // An object is scavengable if its location may move during a scavenge.
  // (A scavenge is a GC which is not a full GC.)
  virtual bool is_scavengable(oop obj);
  // Registering and unregistering an nmethod (compiled code) with the heap.
  // Override with specific mechanism for each specialized heap type.

  // Heap verification
  virtual void verify(VerifyOption option);

  void post_initialize();

};


#endif // SHARE_VM_GC_MMTK_MMTKHEAP_HPP
