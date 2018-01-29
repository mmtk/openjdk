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
    return NULL;
}


jint MMTkHeap::initialize() {
//    jint res =  this->ParallelScavengeHeap::initialize();
//    const size_t heap_size = collector_policy()->max_heap_byte_size();
//   size_t mmtk_heap_size = heap_size;
    CollectedHeap::pre_initialize();
    
    ReservedSpace heap_rs = Universe::reserve_heap(heap_size, _collector_policy->heap_alignment());

    os::trace_page_sizes("Heap",
                       _collector_policy->min_heap_byte_size(),
                       heap_size,
                       generation_alignment(),
                       heap_rs.base(),
                       heap_rs.size());
  
    size_t mmtk_heap_size = 1024*1024*500;
    gc_init(mmtk_heap_size);
    printf("inside mmtkHeap.cpp after initialization with size %d\n", mmtk_heap_size);
    return JNI_OK;
    
}

virtual HeapWord* MMTkHeap::mem_allocate(size_t size, bool* gc_overhead_limit_was_exceeded) {
    
    HeapWord* obj = (HeapWord*) alloc_slow(thread->mmtk_mutator(), size*HeapWordSize, 1, 0);
    printf("inside mmtkHeap.cpp mem_allocating %x, %d\n", obj, size);
    return obj;
}