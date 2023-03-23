## TODOs

These are "manual" patches of the patched CWB code that need to be turned into
an automated patch at a later stage.

cl/regopt.c
-----------

The following lines have been inserted manually.

#ifndef PCRE2_CODE_UNIT_WIDTH
#define PCRE2_CODE_UNIT_WIDTH 8
#endif
#include <pcre2.h>

There are various patches to accomodate the move to pcre2. Most have been 
indicated with a comment /* patched */

