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
      // std::cerr << "Decrementing SV " << value << std::endl;
      SvREFCNT_dec(value);
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
    THIS->insert(ptr, low, high);

AV *
PerlIntervalTree::fetch(int low, int high)
  PROTOTYPE: $;$
  CODE:
    RETVAL = newAV();
    sv_2mortal((SV*)RETVAL);
    std::vector<std::tr1::shared_ptr<SV> > intervals = THIS->fetch(low, high);
    for (size_t i=0; i<intervals.size(); i++) {
      SV *value = intervals[i].get();
      // std::cerr << "refcnt for " << value << " is " << SvREFCNT(value) << std::endl;
      SvREFCNT_inc(value);
      av_push(RETVAL, value);
      // std::cerr << "refcnt for " << value << " is " << SvREFCNT(value) << std::endl;
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
    RETVAL = THIS->high();
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

