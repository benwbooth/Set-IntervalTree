#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

#include <interval_tree.h>

#include "const-c.inc"

MODULE = Set::IntervalTree		PACKAGE = Set::IntervalTree		

INCLUDE: const-xs.inc
