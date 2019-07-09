#include "memory/iterator.hpp"
#include "oops/oop.hpp"
#include "oops/oop.inline.hpp"
#include "../../../../../mmtk/api/mmtk.h"

class MMTkRootsClosure : public OopClosure {
  void* _trace;

  template <class T>
  void do_oop_work(T* p) {
    // T heap_oop = oopDesc::load_heap_oop(p);
    // if (!oopDesc::is_null(heap_oop)) {
    //   oop obj = oopDesc::decode_heap_oop_not_null(heap_oop);
    //   report_delayed_root_edge(_trace, (void*) obj);
    // }
    report_delayed_root_edge(_trace, (void*) p);
  }

public:
  MMTkRootsClosure(void* trace): _trace(trace) {}

  virtual void do_oop(oop* p)       { do_oop_work(p); }
  virtual void do_oop(narrowOop* p) { do_oop_work(p); }
};

class MMTkScanObjectClosure : public ExtendedOopClosure {
  void* _trace;

  template <class T>
  void do_oop_work(T* p) {
    // oop ref = (void*) oopDesc::decode_heap_oop(oopDesc::load_heap_oop(p));
    process_edge(_trace, (void*) p);
  }

public:
  MMTkScanObjectClosure(void* trace): _trace(trace) {}

  virtual void do_oop(oop* p)       { do_oop_work(p); }
  virtual void do_oop(narrowOop* p) { do_oop_work(p); }
};

// class MMTkCLDClosure : public CLDClosure {
// public:
//   virtual void do_cld(ClassLoaderData* cld) {
    
//     printf("CLD: %p", p);
//   }
// };