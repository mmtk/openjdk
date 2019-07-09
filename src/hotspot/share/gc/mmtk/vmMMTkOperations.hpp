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

#ifndef SHARE_VM_GC_MMTK_VMMMTKOPERATIONS_HPP
#define SHARE_VM_GC_MMTK_VMMMTKOPERATIONS_HPP

#include "gc/shared/gcCause.hpp"
#include "gc/shared/vmGCOperations.hpp"
#include "mmtkHeap.hpp"

class VM_MMTkCollect : public VM_Operation {
  MMTkHeap* const _heap;
  const GCCause::Cause _cause;
public:
  VM_MMTkCollect(GCCause::Cause cause);

  virtual VMOp_Type type() const {
    return VMOp_MMTkCollect;
  }
  virtual bool doit_prologue();
  virtual void doit();
  virtual void doit_epilogue();
};

#endif // SHARE_VM_GC_PARALLEL_VMPSOPERATIONS_HPP
