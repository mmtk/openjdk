/*
 * Copyright (c) 2002, 2016, Oracle and/or its affiliates. All rights reserved.
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
#ifdef DISABLING_THE_FILE

#include "precompiled.hpp"
#include "gc/mmtk/mmtkgcTaskManager.hpp"
#include "gc/mmtk/mmtkgcTaskThread.hpp"
#include "gc/shared/gcId.hpp"
#include "gc/shared/workerManager.hpp"
#include "logging/log.hpp"
#include "logging/logStream.hpp"
#include "memory/allocation.hpp"
#include "memory/allocation.inline.hpp"
#include "memory/resourceArea.hpp"
#include "runtime/mutex.hpp"
#include "runtime/mutexLocker.hpp"
#include "runtime/orderAccess.inline.hpp"
#include "runtime/os.hpp"

//
// mmtkGCTask
//

const char* mmtkGCTask::Kind::to_string(kind value) {
  const char* result = "unknown mmtkGCTask kind";
  switch (value) {
  default:
    result = "unknown mmtkGCTask kind";
    break;
  case unknown_task:
    result = "unknown task";
    break;
  case ordinary_task:
    result = "ordinary task";
    break;
  case wait_for_barrier_task:
    result = "wait for barrier task";
    break;
  case noop_task:
    result = "noop task";
    break;
  case idle_task:
    result = "idle task";
    break;
  }
  return result;
};

mmtkGCTask::mmtkGCTask() {
  initialize(Kind::ordinary_task, GCId::current());
}

mmtkGCTask::mmtkGCTask(Kind::kind kind) {
  initialize(kind, GCId::current());
}

mmtkGCTask::mmtkGCTask(Kind::kind kind, uint gc_id) {
  initialize(kind, gc_id);
}

void mmtkGCTask::initialize(Kind::kind kind, uint gc_id) {
  _kind = kind;
  _affinity = mmtkGCTaskManager::sentinel_worker();
  _older = NULL;
  _newer = NULL;
  _gc_id = gc_id;
}

void mmtkGCTask::destruct() {
  assert(older() == NULL, "shouldn't have an older task");
  assert(newer() == NULL, "shouldn't have a newer task");
  // Nothing to do.
}

NOT_PRODUCT(
void mmtkGCTask::print(const char* message) const {
  tty->print(INTPTR_FORMAT " <- " INTPTR_FORMAT "(%u) -> " INTPTR_FORMAT,
             p2i(newer()), p2i(this), affinity(), p2i(older()));
}
)

//
// mmtkGCTaskQueue
//

mmtkGCTaskQueue* mmtkGCTaskQueue::create() {
  mmtkGCTaskQueue* result = new mmtkGCTaskQueue(false);
  if (TraceGCTaskQueue) {
    tty->print_cr("mmtkGCTaskQueue::create()"
                  " returns " INTPTR_FORMAT, p2i(result));
  }
  return result;
}

mmtkGCTaskQueue* mmtkGCTaskQueue::create_on_c_heap() {
  mmtkGCTaskQueue* result = new(ResourceObj::C_HEAP, mtGC) mmtkGCTaskQueue(true);
  if (TraceGCTaskQueue) {
    tty->print_cr("mmtkGCTaskQueue::create_on_c_heap()"
                  " returns " INTPTR_FORMAT,
                  p2i(result));
  }
  return result;
}

mmtkGCTaskQueue::mmtkGCTaskQueue(bool on_c_heap) :
  _is_c_heap_obj(on_c_heap) {
  initialize();
  if (TraceGCTaskQueue) {
    tty->print_cr("[" INTPTR_FORMAT "]"
                  " mmtkGCTaskQueue::mmtkGCTaskQueue() constructor",
                  p2i(this));
  }
}

void mmtkGCTaskQueue::destruct() {
  // Nothing to do.
}

void mmtkGCTaskQueue::destroy(mmtkGCTaskQueue* that) {
  if (TraceGCTaskQueue) {
    tty->print_cr("[" INTPTR_FORMAT "]"
                  " mmtkGCTaskQueue::destroy()"
                  "  is_c_heap_obj:  %s",
                  p2i(that),
                  that->is_c_heap_obj() ? "true" : "false");
  }
  // That instance may have been allocated as a CHeapObj,
  // in which case we have to free it explicitly.
  if (that != NULL) {
    that->destruct();
    assert(that->is_empty(), "should be empty");
    if (that->is_c_heap_obj()) {
      FreeHeap(that);
    }
  }
}

void mmtkGCTaskQueue::initialize() {
  set_insert_end(NULL);
  set_remove_end(NULL);
  set_length(0);
}

// Enqueue one task.
void mmtkGCTaskQueue::enqueue(mmtkGCTask* task) {
  if (TraceGCTaskQueue) {
    tty->print_cr("[" INTPTR_FORMAT "]"
                  " mmtkGCTaskQueue::enqueue(task: "
                  INTPTR_FORMAT ")",
                  p2i(this), p2i(task));
    print("before:");
  }
  assert(task != NULL, "shouldn't have null task");
  assert(task->older() == NULL, "shouldn't be on queue");
  assert(task->newer() == NULL, "shouldn't be on queue");
  task->set_newer(NULL);
  task->set_older(insert_end());
  if (is_empty()) {
    set_remove_end(task);
  } else {
    insert_end()->set_newer(task);
  }
  set_insert_end(task);
  increment_length();
  verify_length();
  if (TraceGCTaskQueue) {
    print("after:");
  }
}

// Enqueue a whole list of tasks.  Empties the argument list.
void mmtkGCTaskQueue::enqueue(mmtkGCTaskQueue* list) {
  if (TraceGCTaskQueue) {
    tty->print_cr("[" INTPTR_FORMAT "]"
                  " mmtkGCTaskQueue::enqueue(list: "
                  INTPTR_FORMAT ")",
                  p2i(this), p2i(list));
    print("before:");
    list->print("list:");
  }
  if (list->is_empty()) {
    // Enqueueing the empty list: nothing to do.
    return;
  }
  uint list_length = list->length();
  if (is_empty()) {
    // Enqueueing to empty list: just acquire elements.
    set_insert_end(list->insert_end());
    set_remove_end(list->remove_end());
    set_length(list_length);
  } else {
    // Prepend argument list to our queue.
    list->remove_end()->set_older(insert_end());
    insert_end()->set_newer(list->remove_end());
    set_insert_end(list->insert_end());
    set_length(length() + list_length);
    // empty the argument list.
  }
  list->initialize();
  if (TraceGCTaskQueue) {
    print("after:");
    list->print("list:");
  }
  verify_length();
}

// Dequeue one task.
mmtkGCTask* mmtkGCTaskQueue::dequeue() {
  if (TraceGCTaskQueue) {
    tty->print_cr("[" INTPTR_FORMAT "]"
                  " mmtkGCTaskQueue::dequeue()", p2i(this));
    print("before:");
  }
  assert(!is_empty(), "shouldn't dequeue from empty list");
  mmtkGCTask* result = remove();
  assert(result != NULL, "shouldn't have NULL task");
  if (TraceGCTaskQueue) {
    tty->print_cr("    return: " INTPTR_FORMAT, p2i(result));
    print("after:");
  }
  return result;
}

// Dequeue one task, preferring one with affinity.
mmtkGCTask* mmtkGCTaskQueue::dequeue(uint affinity) {
  if (TraceGCTaskQueue) {
    tty->print_cr("[" INTPTR_FORMAT "]"
                  " mmtkGCTaskQueue::dequeue(%u)", p2i(this), affinity);
    print("before:");
  }
  assert(!is_empty(), "shouldn't dequeue from empty list");
  // Look down to the next barrier for a task with this affinity.
  mmtkGCTask* result = NULL;
  for (mmtkGCTask* element = remove_end();
       element != NULL;
       element = element->newer()) {
    if (element->is_barrier_task()) {
      // Don't consider barrier tasks, nor past them.
      result = NULL;
      break;
    }
    if (element->affinity() == affinity) {
      result = remove(element);
      break;
    }
  }
  // If we didn't find anything with affinity, just take the next task.
  if (result == NULL) {
    result = remove();
  }
  if (TraceGCTaskQueue) {
    tty->print_cr("    return: " INTPTR_FORMAT, p2i(result));
    print("after:");
  }
  return result;
}

mmtkGCTask* mmtkGCTaskQueue::remove() {
  // Dequeue from remove end.
  mmtkGCTask* result = remove_end();
  assert(result != NULL, "shouldn't have null task");
  assert(result->older() == NULL, "not the remove_end");
  set_remove_end(result->newer());
  if (remove_end() == NULL) {
    assert(insert_end() == result, "not a singleton");
    set_insert_end(NULL);
  } else {
    remove_end()->set_older(NULL);
  }
  result->set_newer(NULL);
  decrement_length();
  assert(result->newer() == NULL, "shouldn't be on queue");
  assert(result->older() == NULL, "shouldn't be on queue");
  verify_length();
  return result;
}

mmtkGCTask* mmtkGCTaskQueue::remove(mmtkGCTask* task) {
  // This is slightly more work, and has slightly fewer asserts
  // than removing from the remove end.
  assert(task != NULL, "shouldn't have null task");
  mmtkGCTask* result = task;
  if (result->newer() != NULL) {
    result->newer()->set_older(result->older());
  } else {
    assert(insert_end() == result, "not youngest");
    set_insert_end(result->older());
  }
  if (result->older() != NULL) {
    result->older()->set_newer(result->newer());
  } else {
    assert(remove_end() == result, "not oldest");
    set_remove_end(result->newer());
  }
  result->set_newer(NULL);
  result->set_older(NULL);
  decrement_length();
  verify_length();
  return result;
}

NOT_PRODUCT(
// Count the elements in the queue and verify the length against
// that count.
void mmtkGCTaskQueue::verify_length() const {
  uint count = 0;
  for (mmtkGCTask* element = insert_end();
       element != NULL;
       element = element->older()) {

    count++;
  }
  assert(count == length(), "Length does not match queue");
}

void mmtkGCTaskQueue::print(const char* message) const {
  tty->print_cr("[" INTPTR_FORMAT "] mmtkGCTaskQueue:"
                "  insert_end: " INTPTR_FORMAT
                "  remove_end: " INTPTR_FORMAT
                "  length:       %d"
                "  %s",
                p2i(this), p2i(insert_end()), p2i(remove_end()), length(), message);
  uint count = 0;
  for (mmtkGCTask* element = insert_end();
       element != NULL;
       element = element->older()) {
    element->print("    ");
    count++;
    tty->cr();
  }
  tty->print("Total tasks: %d", count);
}
)

//
// mmtkSynchronizedGCTaskQueue
//

mmtkSynchronizedGCTaskQueue::mmtkSynchronizedGCTaskQueue(mmtkGCTaskQueue* queue_arg,
                                                 Monitor *       lock_arg) :
  _unsynchronized_queue(queue_arg),
  _lock(lock_arg) {
  assert(unsynchronized_queue() != NULL, "null queue");
  assert(lock() != NULL, "null lock");
}

mmtkSynchronizedGCTaskQueue::~mmtkSynchronizedGCTaskQueue() {
  // Nothing to do.
}

//
// mmtkGCTaskManager
//
mmtkGCTaskManager::mmtkGCTaskManager(uint workers) :
  _workers(workers),
  _active_workers(0),
  _idle_workers(0),
  _created_workers(0) {
  initialize();
}

mmtkGCTaskThread* mmtkGCTaskManager::install_worker(uint t) {
  mmtkGCTaskThread* new_worker = mmtkGCTaskThread::create(this, t, _processor_assignment[t]);
  set_thread(t, new_worker);
  return new_worker;
}

void mmtkGCTaskManager::add_workers(bool initializing) {
  os::ThreadType worker_type = os::pgc_thread;
  uint previous_created_workers = _created_workers;

  _created_workers = WorkerManager::add_workers(this,
                                                _active_workers,
                                                _workers,
                                                _created_workers,
                                                worker_type,
                                                initializing);
  _active_workers = MIN2(_created_workers, _active_workers);

  WorkerManager::log_worker_creation(this, previous_created_workers, _active_workers, _created_workers, initializing);
}

const char* mmtkGCTaskManager::group_name() {
  return "ParGC Thread";
}

void mmtkGCTaskManager::initialize() {
  if (TraceGCTaskManager) {
    tty->print_cr("mmtkGCTaskManager::initialize: workers: %u", workers());
  }
  assert(workers() != 0, "no workers");
  _monitor = new Monitor(Mutex::barrier,                // rank
                         "mmtkGCTaskManager monitor",       // name
                         Mutex::_allow_vm_block_flag,   // allow_vm_block
                         Monitor::_safepoint_check_never);
  // The queue for the mmtkGCTaskManager must be a CHeapObj.
  mmtkGCTaskQueue* unsynchronized_queue = mmtkGCTaskQueue::create_on_c_heap();
  _queue = mmtkSynchronizedGCTaskQueue::create(unsynchronized_queue, lock());
  _noop_task = mmtkNoopGCTask::create_on_c_heap();
  _resource_flag = NEW_C_HEAP_ARRAY(bool, workers(), mtGC);
  {
    // Set up worker threads.
    //     Distribute the workers among the available processors,
    //     unless we were told not to, or if the os doesn't want to.
    _processor_assignment = NEW_C_HEAP_ARRAY(uint, workers(), mtGC);
    if (!BindGCTaskThreadsToCPUs ||
        !os::distribute_processes(workers(), _processor_assignment)) {
      for (uint a = 0; a < workers(); a += 1) {
        _processor_assignment[a] = sentinel_worker();
      }
    }

    _thread = NEW_C_HEAP_ARRAY(mmtkGCTaskThread*, workers(), mtGC);
    _active_workers = ParallelGCThreads;
    if (UseDynamicNumberOfGCThreads && !FLAG_IS_CMDLINE(ParallelGCThreads)) {
      _active_workers = 1U;
    }

    Log(gc, task, thread) log;
    if (log.is_trace()) {
      LogStream ls(log.trace());
      ls.print("mmtkGCTaskManager::initialize: distribution:");
      for (uint t = 0; t < workers(); t += 1) {
        ls.print("  %u", _processor_assignment[t]);
      }
      ls.cr();
    }
  }
  reset_busy_workers();
  set_unblocked();
  for (uint w = 0; w < workers(); w += 1) {
    set_resource_flag(w, false);
  }
  reset_delivered_tasks();
  reset_completed_tasks();
  reset_barriers();
  reset_emptied_queue();

  add_workers(true);
}

mmtkGCTaskManager::~mmtkGCTaskManager() {
  assert(busy_workers() == 0, "still have busy workers");
  assert(queue()->is_empty(), "still have queued work");
  mmtkNoopGCTask::destroy(_noop_task);
  _noop_task = NULL;
  if (_thread != NULL) {
    for (uint i = 0; i < created_workers(); i += 1) {
      mmtkGCTaskThread::destroy(thread(i));
      set_thread(i, NULL);
    }
    FREE_C_HEAP_ARRAY(mmtkGCTaskThread*, _thread);
    _thread = NULL;
  }
  if (_processor_assignment != NULL) {
    FREE_C_HEAP_ARRAY(uint, _processor_assignment);
    _processor_assignment = NULL;
  }
  if (_resource_flag != NULL) {
    FREE_C_HEAP_ARRAY(bool, _resource_flag);
    _resource_flag = NULL;
  }
  if (queue() != NULL) {
    mmtkGCTaskQueue* unsynchronized_queue = queue()->unsynchronized_queue();
    mmtkGCTaskQueue::destroy(unsynchronized_queue);
    mmtkSynchronizedGCTaskQueue::destroy(queue());
    _queue = NULL;
  }
  if (monitor() != NULL) {
    delete monitor();
    _monitor = NULL;
  }
}

void mmtkGCTaskManager::set_active_gang() {
  _active_workers =
    AdaptiveSizePolicy::calc_active_workers(workers(),
                                 active_workers(),
                                 Threads::number_of_non_daemon_threads());

  assert(!all_workers_active() || active_workers() == ParallelGCThreads,
         "all_workers_active() is  incorrect: "
         "active %d  ParallelGCThreads %u", active_workers(),
         ParallelGCThreads);
  _active_workers = MIN2(_active_workers, _workers);
  // "add_workers" does not guarantee any additional workers
  add_workers(false);
  log_trace(gc, task)("mmtkGCTaskManager::set_active_gang(): "
                      "all_workers_active()  %d  workers %d  "
                      "active  %d  ParallelGCThreads %u",
                      all_workers_active(), workers(),  active_workers(),
                      ParallelGCThreads);
}

// Create IdleGCTasks for inactive workers.
// Creates tasks in a ResourceArea and assumes
// an appropriate ResourceMark.
void mmtkGCTaskManager::task_idle_workers() {
  {
    int more_inactive_workers = 0;
    {
      // Stop any idle tasks from exiting their mmtkIdleGCTask's
      // and get the count for additional mmtkIdleGCTask's under
      // the mmtkGCTaskManager's monitor so that the "more_inactive_workers"
      // count is correct.
      MutexLockerEx ml(monitor(), Mutex::_no_safepoint_check_flag);
      _wait_helper.set_should_wait(true);
      // active_workers are a number being requested.  idle_workers
      // are the number currently idle.  If all the workers are being
      // requested to be active but some are already idle, reduce
      // the number of active_workers to be consistent with the
      // number of idle_workers.  The idle_workers are stuck in
      // idle tasks and will no longer be release (since a new GC
      // is starting).  Try later to release enough idle_workers
      // to allow the desired number of active_workers.
      more_inactive_workers =
        created_workers() - active_workers() - idle_workers();
      if (more_inactive_workers < 0) {
        int reduced_active_workers = active_workers() + more_inactive_workers;
        update_active_workers(reduced_active_workers);
        more_inactive_workers = 0;
      }
      log_trace(gc, task)("JT: %d  workers %d  active  %d  idle %d  more %d",
                          Threads::number_of_non_daemon_threads(),
                          created_workers(),
                          active_workers(),
                          idle_workers(),
                          more_inactive_workers);
    }
    mmtkGCTaskQueue* q = mmtkGCTaskQueue::create();
    for(uint i = 0; i < (uint) more_inactive_workers; i++) {
      q->enqueue(mmtkIdleGCTask::create_on_c_heap());
      increment_idle_workers();
    }
    assert(created_workers() == active_workers() + idle_workers(),
      "total workers should equal active + inactive");
    add_list(q);
    // mmtkGCTaskQueue* q was created in a ResourceArea so a
    // destroy() call is not needed.
  }
}

void  mmtkGCTaskManager::release_idle_workers() {
  {
    MutexLockerEx ml(monitor(),
      Mutex::_no_safepoint_check_flag);
    _wait_helper.set_should_wait(false);
    monitor()->notify_all();
  // Release monitor
  }
}

void mmtkGCTaskManager::print_task_time_stamps() {
  if (!log_is_enabled(Debug, gc, task, time)) {
    return;
  }
  uint num_thr = created_workers();
  for(uint i=0; i < num_thr; i++) {
    mmtkGCTaskThread* t = thread(i);
    t->print_task_time_stamps();
  }
}

void mmtkGCTaskManager::print_threads_on(outputStream* st) {
  uint num_thr = created_workers();
  for (uint i = 0; i < num_thr; i++) {
    thread(i)->print_on(st);
    st->cr();
  }
}

void mmtkGCTaskManager::threads_do(ThreadClosure* tc) {
  assert(tc != NULL, "Null ThreadClosure");
  uint num_thr = created_workers();
  for (uint i = 0; i < num_thr; i++) {
    tc->do_thread(thread(i));
  }
}

mmtkGCTaskThread* mmtkGCTaskManager::thread(uint which) {
  assert(which < created_workers(), "index out of bounds");
  assert(_thread[which] != NULL, "shouldn't have null thread");
  return _thread[which];
}

void mmtkGCTaskManager::set_thread(uint which, mmtkGCTaskThread* value) {
  // "_created_workers" may not have been updated yet so use workers()
  assert(which < workers(), "index out of bounds");
  assert(value != NULL, "shouldn't have null thread");
  _thread[which] = value;
}

void mmtkGCTaskManager::add_task(mmtkGCTask* task) {
  assert(task != NULL, "shouldn't have null task");
  MutexLockerEx ml(monitor(), Mutex::_no_safepoint_check_flag);
  if (TraceGCTaskManager) {
    tty->print_cr("mmtkGCTaskManager::add_task(" INTPTR_FORMAT " [%s])",
                  p2i(task), mmtkGCTask::Kind::to_string(task->kind()));
  }
  queue()->enqueue(task);
  // Notify with the lock held to avoid missed notifies.
  if (TraceGCTaskManager) {
    tty->print_cr("    mmtkGCTaskManager::add_task (%s)->notify_all",
                  monitor()->name());
  }
  (void) monitor()->notify_all();
  // Release monitor().
}

void mmtkGCTaskManager::add_list(mmtkGCTaskQueue* list) {
  assert(list != NULL, "shouldn't have null task");
  MutexLockerEx ml(monitor(), Mutex::_no_safepoint_check_flag);
  if (TraceGCTaskManager) {
    tty->print_cr("mmtkGCTaskManager::add_list(%u)", list->length());
  }
  queue()->enqueue(list);
  // Notify with the lock held to avoid missed notifies.
  if (TraceGCTaskManager) {
    tty->print_cr("    mmtkGCTaskManager::add_list (%s)->notify_all",
                  monitor()->name());
  }
  (void) monitor()->notify_all();
  // Release monitor().
}

// GC workers wait in get_task() for new work to be added
// to the mmtkGCTaskManager's queue.  When new work is added,
// a notify is sent to the waiting GC workers which then
// compete to get tasks.  If a GC worker wakes up and there
// is no work on the queue, it is given a noop_task to execute
// and then loops to find more work.

mmtkGCTask* mmtkGCTaskManager::get_task(uint which) {
  mmtkGCTask* result = NULL;
  // Grab the queue lock.
  MutexLockerEx ml(monitor(), Mutex::_no_safepoint_check_flag);
  // Wait while the queue is block or
  // there is nothing to do, except maybe release resources.
  while (is_blocked() ||
         (queue()->is_empty() && !should_release_resources(which))) {
    if (TraceGCTaskManager) {
      tty->print_cr("mmtkGCTaskManager::get_task(%u)"
                    "  blocked: %s"
                    "  empty: %s"
                    "  release: %s",
                    which,
                    is_blocked() ? "true" : "false",
                    queue()->is_empty() ? "true" : "false",
                    should_release_resources(which) ? "true" : "false");
      tty->print_cr("    => (%s)->wait()",
                    monitor()->name());
    }
    monitor()->wait(Mutex::_no_safepoint_check_flag, 0);
  }
  // We've reacquired the queue lock here.
  // Figure out which condition caused us to exit the loop above.
  if (!queue()->is_empty()) {
    if (UseGCTaskAffinity) {
      result = queue()->dequeue(which);
    } else {
      result = queue()->dequeue();
    }
    if (result->is_barrier_task()) {
      assert(which != sentinel_worker(),
             "blocker shouldn't be bogus");
      set_blocking_worker(which);
    }
  } else {
    // The queue is empty, but we were woken up.
    // Just hand back a Noop task,
    // in case someone wanted us to release resources, or whatever.
    result = noop_task();
  }
  assert(result != NULL, "shouldn't have null task");
  if (TraceGCTaskManager) {
    tty->print_cr("mmtkGCTaskManager::get_task(%u) => " INTPTR_FORMAT " [%s]",
                  which, p2i(result), mmtkGCTask::Kind::to_string(result->kind()));
    tty->print_cr("     %s", result->name());
  }
  if (!result->is_idle_task()) {
    increment_busy_workers();
    increment_delivered_tasks();
  }
  return result;
  // Release monitor().
}

void mmtkGCTaskManager::note_completion(uint which) {
  MutexLockerEx ml(monitor(), Mutex::_no_safepoint_check_flag);
  if (TraceGCTaskManager) {
    tty->print_cr("mmtkGCTaskManager::note_completion(%u)", which);
  }
  // If we are blocked, check if the completing thread is the blocker.
  if (blocking_worker() == which) {
    assert(blocking_worker() != sentinel_worker(),
           "blocker shouldn't be bogus");
    increment_barriers();
    set_unblocked();
  }
  increment_completed_tasks();
  uint active = decrement_busy_workers();
  if ((active == 0) && (queue()->is_empty())) {
    increment_emptied_queue();
    if (TraceGCTaskManager) {
      tty->print_cr("    mmtkGCTaskManager::note_completion(%u) done", which);
    }
  }
  if (TraceGCTaskManager) {
    tty->print_cr("    mmtkGCTaskManager::note_completion(%u) (%s)->notify_all",
                  which, monitor()->name());
    tty->print_cr("  "
                  "  blocked: %s"
                  "  empty: %s"
                  "  release: %s",
                  is_blocked() ? "true" : "false",
                  queue()->is_empty() ? "true" : "false",
                  should_release_resources(which) ? "true" : "false");
    tty->print_cr("  "
                  "  delivered: %u"
                  "  completed: %u"
                  "  barriers: %u"
                  "  emptied: %u",
                  delivered_tasks(),
                  completed_tasks(),
                  barriers(),
                  emptied_queue());
  }
  // Tell everyone that a task has completed.
  (void) monitor()->notify_all();
  // Release monitor().
}

uint mmtkGCTaskManager::increment_busy_workers() {
  assert(queue()->own_lock(), "don't own the lock");
  _busy_workers += 1;
  return _busy_workers;
}

uint mmtkGCTaskManager::decrement_busy_workers() {
  assert(queue()->own_lock(), "don't own the lock");
  assert(_busy_workers > 0, "About to make a mistake");
  _busy_workers -= 1;
  return _busy_workers;
}

void mmtkGCTaskManager::release_all_resources() {
  // If you want this to be done atomically, do it in a mmtkWaitForBarrierGCTask.
  for (uint i = 0; i < created_workers(); i += 1) {
    set_resource_flag(i, true);
  }
}

bool mmtkGCTaskManager::should_release_resources(uint which) {
  // This can be done without a lock because each thread reads one element.
  return resource_flag(which);
}

void mmtkGCTaskManager::note_release(uint which) {
  // This can be done without a lock because each thread writes one element.
  set_resource_flag(which, false);
}

// "list" contains tasks that are ready to execute.  Those
// tasks are added to the mmtkGCTaskManager's queue of tasks and
// then the GC workers are notified that there is new work to
// do.
//
// Typically different types of tasks can be added to the "list".
// For example in PSScavenge OldToYoungRootsTask, SerialOldToYoungRootsTask,
// ScavengeRootsTask, and StealTask tasks are all added to the list
// and then the GC workers are notified of new work.  The tasks are
// handed out in the order in which they are added to the list
// (although execution is not necessarily in that order).  As long
// as any tasks are running the mmtkGCTaskManager will wait for execution
// to complete.  GC workers that execute a stealing task remain in
// the stealing task until all stealing tasks have completed.  The load
// balancing afforded by the stealing tasks work best if the stealing
// tasks are added last to the list.

void mmtkGCTaskManager::execute_and_wait(mmtkGCTaskQueue* list) {
  mmtkWaitForBarrierGCTask* fin = mmtkWaitForBarrierGCTask::create();
  list->enqueue(fin);
  // The barrier task will be read by one of the GC
  // workers once it is added to the list of tasks.
  // Be sure that is globally visible before the
  // GC worker reads it (which is after the task is added
  // to the list of tasks below).
  OrderAccess::storestore();
  add_list(list);
  fin->wait_for(true /* reset */);
  // We have to release the barrier tasks!
  mmtkWaitForBarrierGCTask::destroy(fin);
}

bool mmtkGCTaskManager::resource_flag(uint which) {
  assert(which < workers(), "index out of bounds");
  return _resource_flag[which];
}

void mmtkGCTaskManager::set_resource_flag(uint which, bool value) {
  assert(which < workers(), "index out of bounds");
  _resource_flag[which] = value;
}

//
// mmtkNoopGCTask
//

mmtkNoopGCTask* mmtkNoopGCTask::create_on_c_heap() {
  mmtkNoopGCTask* result = new(ResourceObj::C_HEAP, mtGC) mmtkNoopGCTask();
  return result;
}

void mmtkNoopGCTask::destroy(mmtkNoopGCTask* that) {
  if (that != NULL) {
    that->destruct();
    FreeHeap(that);
  }
}

// This task should never be performing GC work that require
// a valid GC id.
mmtkNoopGCTask::mmtkNoopGCTask() : mmtkGCTask(mmtkGCTask::Kind::noop_task, GCId::undefined()) { }

void mmtkNoopGCTask::destruct() {
  // This has to know it's superclass structure, just like the constructor.
  this->mmtkGCTask::destruct();
  // Nothing else to do.
}

//
// mmtkIdleGCTask
//

mmtkIdleGCTask* mmtkIdleGCTask::create() {
  mmtkIdleGCTask* result = new mmtkIdleGCTask(false);
  assert(UseDynamicNumberOfGCThreads,
    "Should only be used with dynamic GC thread");
  return result;
}

mmtkIdleGCTask* mmtkIdleGCTask::create_on_c_heap() {
  mmtkIdleGCTask* result = new(ResourceObj::C_HEAP, mtGC) mmtkIdleGCTask(true);
  assert(UseDynamicNumberOfGCThreads,
    "Should only be used with dynamic GC thread");
  return result;
}

void mmtkIdleGCTask::do_it(mmtkGCTaskManager* manager, uint which) {
  WaitHelper* wait_helper = manager->wait_helper();
  log_trace(gc, task)("[" INTPTR_FORMAT "] mmtkIdleGCTask:::do_it() should_wait: %s",
      p2i(this), wait_helper->should_wait() ? "true" : "false");

  MutexLockerEx ml(manager->monitor(), Mutex::_no_safepoint_check_flag);
  log_trace(gc, task)("--- idle %d", which);
  // Increment has to be done when the idle tasks are created.
  // manager->increment_idle_workers();
  manager->monitor()->notify_all();
  while (wait_helper->should_wait()) {
    log_trace(gc, task)("[" INTPTR_FORMAT "] mmtkIdleGCTask::do_it()  [" INTPTR_FORMAT "] (%s)->wait()",
      p2i(this), p2i(manager->monitor()), manager->monitor()->name());
    manager->monitor()->wait(Mutex::_no_safepoint_check_flag, 0);
  }
  manager->decrement_idle_workers();

  log_trace(gc, task)("--- release %d", which);
  log_trace(gc, task)("[" INTPTR_FORMAT "] mmtkIdleGCTask::do_it() returns should_wait: %s",
    p2i(this), wait_helper->should_wait() ? "true" : "false");
  // Release monitor().
}

void mmtkIdleGCTask::destroy(mmtkIdleGCTask* that) {
  if (that != NULL) {
    that->destruct();
    if (that->is_c_heap_obj()) {
      FreeHeap(that);
    }
  }
}

void mmtkIdleGCTask::destruct() {
  // This has to know it's superclass structure, just like the constructor.
  this->mmtkGCTask::destruct();
  // Nothing else to do.
}

//
// mmtkWaitForBarrierGCTask
//
mmtkWaitForBarrierGCTask* mmtkWaitForBarrierGCTask::create() {
  mmtkWaitForBarrierGCTask* result = new mmtkWaitForBarrierGCTask();
  return result;
}

mmtkWaitForBarrierGCTask::mmtkWaitForBarrierGCTask() : mmtkGCTask(mmtkGCTask::Kind::wait_for_barrier_task) { }

void mmtkWaitForBarrierGCTask::destroy(mmtkWaitForBarrierGCTask* that) {
  if (that != NULL) {
    if (TraceGCTaskManager) {
      tty->print_cr("[" INTPTR_FORMAT "] mmtkWaitForBarrierGCTask::destroy()", p2i(that));
    }
    that->destruct();
  }
}

void mmtkWaitForBarrierGCTask::destruct() {
  if (TraceGCTaskManager) {
    tty->print_cr("[" INTPTR_FORMAT "] mmtkWaitForBarrierGCTask::destruct()", p2i(this));
  }
  this->mmtkGCTask::destruct();
  // Clean up that should be in the destructor,
  // except that ResourceMarks don't call destructors.
  _wait_helper.release_monitor();
}

void mmtkWaitForBarrierGCTask::do_it_internal(mmtkGCTaskManager* manager, uint which) {
  // Wait for this to be the only busy worker.
  assert(manager->monitor()->owned_by_self(), "don't own the lock");
  assert(manager->is_blocked(), "manager isn't blocked");
  while (manager->busy_workers() > 1) {
    if (TraceGCTaskManager) {
      tty->print_cr("mmtkWaitForBarrierGCTask::do_it(%u) waiting on %u workers",
                    which, manager->busy_workers());
    }
    manager->monitor()->wait(Mutex::_no_safepoint_check_flag, 0);
  }
}

void mmtkWaitForBarrierGCTask::do_it(mmtkGCTaskManager* manager, uint which) {
  if (TraceGCTaskManager) {
    tty->print_cr("[" INTPTR_FORMAT "]"
                  " mmtkWaitForBarrierGCTask::do_it() waiting for idle",
                  p2i(this));
  }
  {
    // First, wait for the barrier to arrive.
    MutexLockerEx ml(manager->lock(), Mutex::_no_safepoint_check_flag);
    do_it_internal(manager, which);
    // Release manager->lock().
  }
  // Then notify the waiter.
  _wait_helper.notify();
}

WaitHelper::WaitHelper() : _should_wait(true), _monitor(mmtkMonitorSupply::reserve()) {
  if (TraceGCTaskManager) {
    tty->print_cr("[" INTPTR_FORMAT "]"
                  " WaitHelper::WaitHelper()"
                  "  monitor: " INTPTR_FORMAT,
                  p2i(this), p2i(monitor()));
  }
}

void WaitHelper::release_monitor() {
  assert(_monitor != NULL, "");
  mmtkMonitorSupply::release(_monitor);
  _monitor = NULL;
}

WaitHelper::~WaitHelper() {
  release_monitor();
}

void WaitHelper::wait_for(bool reset) {
  if (TraceGCTaskManager) {
    tty->print_cr("[" INTPTR_FORMAT "]"
                  " mmtkWaitForBarrierGCTask::wait_for()"
      "  should_wait: %s",
      p2i(this), should_wait() ? "true" : "false");
  }
  {
    // Grab the lock and check again.
    MutexLockerEx ml(monitor(), Mutex::_no_safepoint_check_flag);
    while (should_wait()) {
      if (TraceGCTaskManager) {
        tty->print_cr("[" INTPTR_FORMAT "]"
                      " mmtkWaitForBarrierGCTask::wait_for()"
          "  [" INTPTR_FORMAT "] (%s)->wait()",
          p2i(this), p2i(monitor()), monitor()->name());
      }
      monitor()->wait(Mutex::_no_safepoint_check_flag, 0);
    }
    // Reset the flag in case someone reuses this task.
    if (reset) {
      set_should_wait(true);
    }
    if (TraceGCTaskManager) {
      tty->print_cr("[" INTPTR_FORMAT "]"
                    " mmtkWaitForBarrierGCTask::wait_for() returns"
        "  should_wait: %s",
        p2i(this), should_wait() ? "true" : "false");
    }
    // Release monitor().
  }
}

void WaitHelper::notify() {
  MutexLockerEx ml(monitor(), Mutex::_no_safepoint_check_flag);
  set_should_wait(false);
  // Waiter doesn't miss the notify in the wait_for method
  // since it checks the flag after grabbing the monitor.
  if (TraceGCTaskManager) {
    tty->print_cr("[" INTPTR_FORMAT "]"
                  " mmtkWaitForBarrierGCTask::do_it()"
                  "  [" INTPTR_FORMAT "] (%s)->notify_all()",
                  p2i(this), p2i(monitor()), monitor()->name());
  }
  monitor()->notify_all();
}

Mutex*                   mmtkMonitorSupply::_lock     = NULL;
GrowableArray<Monitor*>* mmtkMonitorSupply::_freelist = NULL;

Monitor* mmtkMonitorSupply::reserve() {
  Monitor* result = NULL;
  // Lazy initialization: possible race.
  if (lock() == NULL) {
    _lock = new Mutex(Mutex::barrier,                  // rank
                      "mmtkMonitorSupply mutex",           // name
                      Mutex::_allow_vm_block_flag);    // allow_vm_block
  }
  {
    MutexLockerEx ml(lock());
    // Lazy initialization.
    if (freelist() == NULL) {
      _freelist =
        new(ResourceObj::C_HEAP, mtGC) GrowableArray<Monitor*>(ParallelGCThreads,
                                                         true);
    }
    if (! freelist()->is_empty()) {
      result = freelist()->pop();
    } else {
      result = new Monitor(Mutex::barrier,                  // rank
                           "mmtkMonitorSupply monitor",         // name
                           Mutex::_allow_vm_block_flag,     // allow_vm_block
                           Monitor::_safepoint_check_never);
    }
    guarantee(result != NULL, "shouldn't return NULL");
    assert(!result->is_locked(), "shouldn't be locked");
    // release lock().
  }
  return result;
}

void mmtkMonitorSupply::release(Monitor* instance) {
  assert(instance != NULL, "shouldn't release NULL");
  assert(!instance->is_locked(), "shouldn't be locked");
  {
    MutexLockerEx ml(lock());
    freelist()->push(instance);
    // release lock().
  }
}


#endif //DISABLING_THE_FILE