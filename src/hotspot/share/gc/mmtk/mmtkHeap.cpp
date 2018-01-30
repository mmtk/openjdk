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

#include "precompiled.hpp"
#include "code/codeCache.hpp"
#include "gc/mmtk/mmtkHeap.inline.hpp"
#include "gc/shared/gcHeapSummary.hpp"
#include "gc/shared/gcLocker.inline.hpp"
#include "gc/shared/gcWhen.hpp"
#include "logging/log.hpp"
#include "oops/oop.inline.hpp"
#include "runtime/handles.inline.hpp"
#include "runtime/java.hpp"
#include "runtime/vmThread.hpp"
#include "services/memoryManager.hpp"
#include "services/memTracker.hpp"
#include "utilities/vmError.hpp"
#include "../../../../../mmtk/api/mmtk.h"


HeapWord* MMTkHeap::allocate_from_tlab(Klass* klass, Thread* thread, size_t size) {
   

    void* obj_ptr = alloc(thread->mmtk_mutator(), size*HeapWordSize, 1, 0);
    HeapWord* obj = (HeapWord*) obj_ptr;
     
    if (obj != NULL) {
       printf("inside mmtkHeap.cpp allocated from mmtk %x, %x, %d\n", obj, obj_ptr, size);
      return obj;
    }
    // Otherwise...
    printf("inside mmtkHeap.cpp returned NULL\n");
    return (HeapWord*) alloc_slow(thread->mmtk_mutator(), size*HeapWordSize, 1, 0);
}


jint MMTkHeap::initialize() {
    
   CollectedHeap::pre_initialize();
    
    const size_t heap_size = collector_policy()->max_heap_byte_size();
   size_t mmtk_heap_size = heap_size;
   /*forcefully*/ mmtk_heap_size = 2*1024*1024*1024 -1;
    
    gc_init(mmtk_heap_size);
    printf("inside mmtkHeap.cpp after initialization with size %d\n", mmtk_heap_size);
    return JNI_OK;
    
}

HeapWord* MMTkHeap::mem_allocate(size_t size, bool* gc_overhead_limit_was_exceeded) {
    
    printf("inside mmtkHeap.cpp mem_allocating %d\n", size);
    return NULL;
}


void MMTkHeap::post_initialize() {
  CollectedHeap::post_initialize();
}




////Previously pure abstract methods--

 size_t MMTkHeap::capacity() const { guarantee(false, "capacity not supported"); return 0;}
   size_t MMTkHeap::used() const { guarantee(false, "used not supported"); return 0;}
  
   bool MMTkHeap::is_maximal_no_gc() const { guarantee(false, "is_maximal_no_gc  not supported"); return false;}

   size_t MMTkHeap::max_capacity() const {guarantee(false, "max capacity not supported"); return 0;}
   bool MMTkHeap::is_in(const void* p) const {guarantee(false, "is in not supported"); return false; }
  
    bool MMTkHeap::supports_tlab_allocation() const {guarantee(false, "supports-tlab-allocation buffers not supported"); return false;}

  // The amount of space available for thread-local allocation buffers.
   size_t MMTkHeap::tlab_capacity(Thread *thr) const {guarantee(false, "tlab_capacity not supported");return 0;}

  // The amount of used space for thread-local allocation buffers for the given thread.
   size_t MMTkHeap::tlab_used(Thread *thr) const { guarantee(false, "tlab_used not supported"); return 0;}
  
  
   // Can a compiler initialize a new object without store barriers?
  // This permission only extends from the creation of a new object
  // via a TLAB up to the first subsequent safepoint. //However, we will not use tlab
   bool MMTkHeap::can_elide_tlab_store_barriers() const {
      // guarantee(false, "can elide tlab store barriers not supported"); 
       return true;
   }


   bool MMTkHeap::can_elide_initializing_store_barrier(oop new_obj) {guarantee(false, "can elide initializing store barrier not supported");return false;}
  
  // mark to be thus strictly sequenced after the stores.
   bool MMTkHeap::card_mark_must_follow_store() const {
       //guarantee(false, "card mark must follow store not supported");
       return false;
   }

   void MMTkHeap::collect(GCCause::Cause cause) {guarantee(false, "collect not supported");}

  // Perform a full collection
   void MMTkHeap::do_full_collection(bool clear_all_soft_refs) {guarantee(false, "do full collection not supported");}


  // Return the CollectorPolicy for the heap
   CollectorPolicy* MMTkHeap::collector_policy() const {return _collector_policy;}

   GrowableArray<GCMemoryManager*> MMTkHeap::memory_managers() {
      guarantee(false, "memory managers not supported");
    GrowableArray<GCMemoryManager*> memory_managers(0);
     return memory_managers;
  
  }
   GrowableArray<MemoryPool*> MMTkHeap::memory_pools() {
      guarantee(false, "memory pools not supported");
      GrowableArray<MemoryPool*> memory_pools(0);
    return memory_pools;
  }

  // Iterate over all objects, calling "cl.do_object" on each.
   void MMTkHeap::object_iterate(ObjectClosure* cl) { guarantee(false, "object iterate not supported");}

  // Similar to object_iterate() except iterates only
  // over live objects.
   void MMTkHeap::safe_object_iterate(ObjectClosure* cl) { guarantee(false, "safe object iterate not supported");}

   HeapWord* MMTkHeap::block_start(const void* addr) const {guarantee(false, "block start not supported"); return NULL;}

   size_t MMTkHeap::block_size(const HeapWord* addr) const { guarantee(false, "block size not supported"); return 0; }

   bool MMTkHeap::block_is_obj(const HeapWord* addr) const { guarantee(false, "block is obj not supported"); return false; }

   jlong MMTkHeap::millis_since_last_gc() {guarantee(false, "time since last gc not supported"); return 0; }


   void MMTkHeap::prepare_for_verify() {guarantee(false, "prepare for verify not supported");}


   void MMTkHeap::initialize_serviceability() {guarantee(false, "initialize serviceability not supported"); }
  
  // Print heap information on the given outputStream.
   void MMTkHeap::print_on(outputStream* st) const {guarantee(false, "print on not supported");}


  // Print all GC threads (other than the VM thread)
  // used by this heap.
   void MMTkHeap::print_gc_threads_on(outputStream* st) const {guarantee(false, "print gc threads on not supported");}

  // Iterator for all GC threads (other than VM thread)
   void MMTkHeap::gc_threads_do(ThreadClosure* tc) const {guarantee(false, "gc threads do not supported");}

  // Print any relevant tracing info that flags imply.
  // Default implementation does nothing.
   void MMTkHeap::print_tracing_info() const {guarantee(false, "paint tracing info not supported");}


  // An object is scavengable if its location may move during a scavenge.
  // (A scavenge is a GC which is not a full GC.)
   bool MMTkHeap::is_scavengable(oop obj) {return false;}
  // Registering and unregistering an nmethod (compiled code) with the heap.
  // Override with specific mechanism for each specialized heap type.

  // Heap verification
   void MMTkHeap::verify(VerifyOption option) {guarantee(false, "tverify not supported");}