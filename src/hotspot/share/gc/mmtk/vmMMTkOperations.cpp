/*
 * Copyright (c) 2007, 2015, Oracle and/or its affiliates. All rights reserved.
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
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#include "precompiled.hpp"
#include "gc/mmtk/vmMMTkOperations.hpp"
#include "gc/shared/gcLocker.inline.hpp"
#include "utilities/dtrace.hpp"
#include "mmtkHeap.hpp"

// The following methods are used by the parallel scavenge collector
VM_MMTkCollect::VM_MMTkCollect(GCCause::Cause cause)
  : VM_Operation(), _cause(cause), _heap(MMTkHeap::heap()) {}

bool VM_MMTkCollect::doit_prologue() {
  printf("VM_MMTkCollect::doit_prologue Start\n");
  Heap_lock->lock();
  printf("VM_MMTkCollect::doit_prologue End\n");
  return true;
}

void VM_MMTkCollect::doit() {
  printf("Start  VM_MMTkCollect\n");
  while(true) {}
  // if (GCLocker::is_active_and_needs_gc()) {
  //   printf("Lock GC\n");
  //   set_gc_locked();
  //   printf("GC Locked\n");
  // }
  printf("Finish VM_MMTkCollect\n");
}

void VM_MMTkCollect::doit_epilogue() {
  printf("VM_MMTkCollect::doit_epilogue Start\n");
  Heap_lock->unlock();
  printf("VM_MMTkCollect::doit_epilogue End\n");
}