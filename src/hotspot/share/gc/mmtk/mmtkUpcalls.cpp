/*
 * Copyright (c) 2017, Red Hat, Inc. and/or its affiliates.
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
 * Please contact Sun 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

/* 
 * File:   mmtkUpcalls.cpp
 * Author: Pavel Zakopaylo
 *
 * Created on 30 November 2018, 5:45 PM
 */

#include "mmtkUpcalls.hpp"
#include "mmtkRootsClosure.hpp"
#include "mmtkHeap.hpp"
#include "vmMMTkOperations.hpp"
#include "contextThread.hpp"
#include "mmtkCollectorThread.hpp"
#include "runtime/os.hpp"
#include "runtime/vmThread.hpp"
#include "runtime/safepoint.hpp"
#include "runtime/mutexLocker.hpp"
#include "runtime/thread.hpp"
#include "runtime/threadSMR.hpp"

static bool gcInProgress = false;

static void mmtk_stop_all_mutators(void *tls) {
    printf("mmtk_stop_all_mutators start\n");
    SafepointSynchronize::begin();
    // Heap_lock->lock();
    gcInProgress = true;
    // Heap_lock->unlock();
    printf("mmtk_stop_all_mutators end\n");
}

static void mmtk_resume_mutators(void *tls) {
    printf("mmtk_resume_mutators start\n");
    SafepointSynchronize::end();
    Heap_lock->lock();
    gcInProgress = false;
    Heap_lock->notify_all();
    Heap_lock->unlock();
    printf("mmtk_resume_mutators end\n");
}

static void mmtk_spawn_collector_thread(void* tls, void* ctx) {
    if (ctx == NULL) {
        MMTkContextThread* t = new MMTkContextThread();
        if (!os::create_thread(t, os::pgc_thread)) {
            printf("Failed to create thread");
            guarantee(false, "panic");
        }
        os::start_thread(t);
    } else {
        MMTkCollectorThread* t = new MMTkCollectorThread(ctx);
        if (!os::create_thread(t, os::pgc_thread)) {
            printf("Failed to create thread");
            guarantee(false, "panic");
        }
        os::start_thread(t);
    }
}

static void mmtk_block_for_gc() {
    printf("mmtk_block_for_gc start %p\n", Thread::current());
    do {
        Heap_lock->lock();
        Heap_lock->wait();
        Heap_lock->unlock();
    } while (gcInProgress);
    printf("mmtk_block_for_gc end\n");
    // exit(-1);
}

static void* mmtk_active_collector(void* tls) {
    return ((MMTkCollectorThread*) tls)->get_context();
}

static JavaThread* _thread_cursor = NULL;

static void* mmtk_get_next_mutator() {
    // printf("number_of_threads %d\n", Threads::number_of_threads());
    // printf("number_of_non_daemon_threads %d\n", Threads::number_of_non_daemon_threads());
    if (_thread_cursor == NULL) {
        _thread_cursor = Threads::get_thread_list();
    } else {
        _thread_cursor = _thread_cursor->next();
    }
    // printf("_thread_cursor %p -> %p\n", _thread_cursor, _thread_cursor == NULL ? NULL : _thread_cursor->mmtk_mutator());
    if (_thread_cursor == NULL) return NULL;
    void* mutator = _thread_cursor->mmtk_mutator();
    if (!mutator || (mutator == (void *)0xf1f1f1f1f1f1f1f1)){
        printf("Setting mutator for thread %p\n", _thread_cursor);
        _thread_cursor->set_mmtk_mutator();
    }
    return _thread_cursor->mmtk_mutator();
}

static void mmtk_reset_mutator_iterator() {
    _thread_cursor = NULL;
}


static void mmtk_compute_thread_roots(void* trace, void* tls) {
    MMTkRootsClosure cl(trace);
    MMTkHeap::heap()->scan_roots(cl);
}

static void mmtk_scan_object(void* trace, void* object, void* tls) {
    MMTkScanObjectClosure cl(trace);
    ((oop) object)->oop_iterate(&cl);
}

static void mmtk_dump_object(void* object) {
    oop o = (oop) object;

    // o->print();
    o->print_value();
    printf("\n");
    
    // o->print_address();
}

static size_t mmtk_get_object_size(void* object) {
    oop o = (oop) object;
    return o->size() * HeapWordSize;
}

OpenJDK_Upcalls mmtk_upcalls = {
    mmtk_stop_all_mutators,
    mmtk_resume_mutators,
    mmtk_spawn_collector_thread,
    mmtk_block_for_gc,
    mmtk_active_collector,
    mmtk_get_next_mutator,
    mmtk_reset_mutator_iterator,
    mmtk_compute_thread_roots,
    mmtk_scan_object,
    mmtk_dump_object,
    mmtk_get_object_size,
};