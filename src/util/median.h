#include <inttypes.h>

#include "xutil.h"

/* http://ndevilla.free.fr/median/median/index.html */

/*
 *  This Quickselect routine is based on the algorithm described in
 *  "Numerical recipes in C", Second Edition,
 *  Cambridge University Press, 1992, Section 8.5, ISBN 0-521-43108-5
 *  This code by Nicolas Devillard - 1998. Public domain.
 */

#define SWAP(a, b) { register int64_t t=(a);(a)=(b);(b)=t; }

int64_t median_quick_select(int64_t *restrict v, const uint32_t n);

/*
 * Algorithm from N. Wirth's book, implementation by N. Devillard.
 * This code is public domain.
 */

/*---------------------------------------------------------------------------
   Function :   wirth()
   In       :   array of elements, # of elements in the array
   Out      :   one element
   Job      :   find the median element in the array

                Reference:

                  Author: Wirth, Niklaus 
                   Title: Algorithms + data structures = programs 
               Publisher: Englewood Cliffs: Prentice-Hall, 1976 
    Physical description: 366 p. 
                  Series: Prentice-Hall Series in Automatic Computation 

 ---------------------------------------------------------------------------*/

int64_t median_wirth(int64_t *restrict v, const uint32_t n);
