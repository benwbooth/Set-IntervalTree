#ifdef __cplusplus
extern "C" {
#endif

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#ifdef __cplusplus
}
#endif

#include <tr1/memory>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#include <interval_tree.h>

std::ostream& operator<<(std::ostream &out, const std::tr1::shared_ptr<SV> &value) {
  out << "Node:" << value.get();
  return out;
}

class SV_deleter {
  public:
    void operator() (SV *value) {
      SvREFCNT_dec(value);
    }
};

class RemoveFunctor {
  SV *callback;
  public:
    RemoveFunctor(SV *callback_) : callback(callback_) {}
    bool operator()(std::tr1::shared_ptr<SV> value, int low, int high) const {
      // pass args into callback
      dSP;
      ENTER;
      SAVETMPS;
      PUSHMARK(SP);
      XPUSHs(value.get());
      XPUSHs(sv_2mortal(newSViv(low)));
      XPUSHs(sv_2mortal(newSViv(high+1)));
      PUTBACK;

      // get result from callback and return
      I32 count = call_sv(callback, G_SCALAR);

      SPAGAIN;

      if (count < 1) {
        PUTBACK;
        FREETMPS;
        LEAVE;
        return false;
      }

      SV *retval_sv = POPs;
      bool retval = SvTRUE(retval_sv);

      PUTBACK;
      FREETMPS;
      LEAVE;
      return retval;
    }
};

typedef IntervalTree<std::tr1::shared_ptr<SV> > PerlIntervalTree;
typedef IntervalTree<std::tr1::shared_ptr<SV> >::Node PerlIntervalTree_Node;

MODULE = Set::IntervalTree PACKAGE = Set::IntervalTree

PerlIntervalTree *
PerlIntervalTree::new()

SV *
PerlIntervalTree::str()
  CODE:
    std::string str = THIS->str();
    const char *tree = str.c_str();
    RETVAL = newSVpv(tree, 0);
  OUTPUT:
    RETVAL

void
PerlIntervalTree::insert(SV *value, int low, int high)
  PROTOTYPE: $;$;$
  CODE: 
    SvREFCNT_inc(value);
    std::tr1::shared_ptr<SV> ptr(value, SV_deleter());
    THIS->insert(ptr, low, high-1);

AV *
PerlIntervalTree::remove(int low, int high, ...)
  CODE:
    RETVAL = newAV();
    sv_2mortal((SV*)RETVAL);

    if (items > 3) {
      SV *callback = ST(3); 
      RemoveFunctor remove_functor(callback);
      std::vector<std::tr1::shared_ptr<SV> > removed;
      THIS->remove(low, high-1, remove_functor, removed);

      for (std::vector<std::tr1::shared_ptr<SV> >::const_iterator
          i=removed.begin(); i!=removed.end(); ++i) 
      {
        SV *value = i->get();
        SvREFCNT_inc(value);
        av_push(RETVAL, value);
      }
    }
    else {
      std::vector<std::tr1::shared_ptr<SV> > removed; 
      THIS->remove(low, high-1, removed);

      for (std::vector<std::tr1::shared_ptr<SV> >::const_iterator
          i=removed.begin(); i!=removed.end(); ++i) 
      {
        SV *value = i->get();
        SvREFCNT_inc(value);
        av_push(RETVAL, value);
      }
    }
  OUTPUT:
    RETVAL

AV *
PerlIntervalTree::fetch(int low, int high)
  PROTOTYPE: $;$
  CODE:
    RETVAL = newAV();
    sv_2mortal((SV*)RETVAL);
    std::vector<std::tr1::shared_ptr<SV> > intervals;
    THIS->fetch(low, high-1, intervals);
    for (size_t i=0; i<intervals.size(); i++) {
      SV *value = intervals[i].get();
      SvREFCNT_inc(value);
      av_push(RETVAL, value);
    }
  OUTPUT:
    RETVAL

AV *
PerlIntervalTree::fetch_window(int low, int high)
  PROTOTYPE: $;$
  CODE:
    RETVAL = newAV();
    sv_2mortal((SV*)RETVAL);
    std::vector<std::tr1::shared_ptr<SV> > intervals;
    THIS->fetch_window(low, high-1, intervals);
    for (size_t i=0; i<intervals.size(); i++) {
      SV *value = intervals[i].get();
      SvREFCNT_inc(value);
      av_push(RETVAL, value);
    }
  OUTPUT:
    RETVAL

void 
PerlIntervalTree::DESTROY()

MODULE = Set::IntervalTree PACKAGE = Set::IntervalTree::Node

PerlIntervalTree_Node *
PerlIntervalTree_Node::new()

int
PerlIntervalTree_Node::low()
  CODE:
    RETVAL = THIS->low();
  OUTPUT:
    RETVAL

int
PerlIntervalTree_Node::high()
  CODE:
    RETVAL = THIS->high()+1;
  OUTPUT:
    RETVAL

SV *
PerlIntervalTree_Node::value()
  CODE:
    RETVAL = THIS->value().get();
  OUTPUT:
    RETVAL

void 
PerlIntervalTree_Node::DESTROY()

