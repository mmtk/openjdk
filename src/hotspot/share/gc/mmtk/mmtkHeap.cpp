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
#include "gc/parallel/adjoiningGenerations.hpp"
#include "gc/parallel/adjoiningVirtualSpaces.hpp"
#include "gc/parallel/cardTableExtension.hpp"
#include "gc/parallel/gcTaskManager.hpp"
#include "gc/parallel/generationSizer.hpp"
#include "gc/parallel/objectStartArray.inline.hpp"
#include "gc/mmtk/mmtkHeap.inline.hpp"
#include "gc/parallel/psAdaptiveSizePolicy.hpp"
#include "gc/parallel/psMarkSweep.hpp"
#include "gc/parallel/psMemoryPool.hpp"
#include "gc/parallel/psParallelCompact.inline.hpp"
#include "gc/parallel/psPromotionManager.hpp"
#include "gc/parallel/psScavenge.hpp"
#include "gc/parallel/vmPSOperations.hpp"
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
   
    // extern void* alloc(MMTk_Mutator mutator, size_t size, size_t align, ssize_t offset);
     HeapWord* obj = (HeapWord*)  alloc(thread->mmtk_mutator(), size, 1, 0);
     printf("inside mmtkHeap.cpp allocated from mmtk\n");
    //HeapWord* obj = thread->tlab().allocate(size);
    if (obj != NULL) {
      return obj;
    }
    // Otherwise...
    return (HeapWord*) alloc_slow(thread->mmtk_mutator(), size, 1, 0);
}


jint MMTkHeap::initialize() {
    jint res =  this->ParallelScavengeHeap::initialize();
    const size_t heap_size = collector_policy()->max_heap_byte_size();
    gc_init(heap_size);
    printf("inside mmtkHeap.cpp after initialization\n");
    return res;
    
}