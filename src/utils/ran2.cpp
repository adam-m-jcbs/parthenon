//========================================================================================
// Athena++ astrophysical MHD code
// Copyright(C) 2014 James M. Stone <jmstone@princeton.edu> and other code contributors
// Licensed under the 3-clause BSD License, see LICENSE file for details
//========================================================================================
// (C) (or copyright) 2020. Triad National Security, LLC. All rights reserved.
//
// This program was produced under U.S. Government contract 89233218CNA000001 for Los
// Alamos National Laboratory (LANL), which is operated by Triad National Security, LLC
// for the U.S. Department of Energy/National Nuclear Security Administration. All rights
// in the program are reserved by Triad National Security, LLC, and the U.S. Department
// of Energy/National Nuclear Security Administration. The Government is granted for
// itself and others acting on its behalf a nonexclusive, paid-up, irrevocable worldwide
// license in this material to reproduce, prepare derivative works, distribute copies to
// the public, perform publicly and display publicly, and to permit others to do so.
//========================================================================================
//! \file ran2.cpp

#include <cfloat>
#include <iostream>

#include "athena.hpp"

//----------------------------------------------------------------------------------------
//! \fn double ran2(std::int64_t *idum)
//  \brief  Extracted from the Numerical Recipes in C (version 2) code. Modified
//   to use doubles instead of floats. -- T. A. Gardiner -- Aug. 12, 2003
//
// Long period (> 2 x 10^{18}) random number generator of L'Ecuyer with Bays-Durham
// shuffle and added safeguards.  Returns a uniform random deviate between 0.0 and 1.0
// (exclusive of the endpoint values).  Call with idum = a negative integer to
// initialize; thereafter, do not alter idum between successive deviates in a sequence.
// RNMX should appriximate the largest floating-point value that is less than 1.

#define IMR1 2147483563
#define IMR2 2147483399
#define AM (1.0 / IMR1)
#define IMM1 (IMR1 - 1)
#define IA1 40014
#define IA2 40692
#define IQ1 53668
#define IQ2 52774
#define IR1 12211
#define IR2 3791
#define NTAB 32
#define NDIV (1 + IMM1 / NTAB)
#define RNMX (1.0 - DBL_EPSILON)

namespace parthenon {

double ran2(std::int64_t *idum) {
  int j;
  std::int64_t k;
  static std::int64_t idum2 = 123456789;
  static std::int64_t iy = 0;
  static std::int64_t iv[NTAB];
  double temp;

  if (*idum <= 0) { // Initialize
    if (-(*idum) < 1)
      *idum = 1; // Be sure to prevent idum = 0
    else
      *idum = -(*idum);
    idum2 = (*idum);
    for (j = NTAB + 7; j >= 0; j--) { // Load the shuffle table (after 8 warm-ups)
      k = (*idum) / IQ1;
      *idum = IA1 * (*idum - k * IQ1) - k * IR1;
      if (*idum < 0) *idum += IMR1;
      if (j < NTAB) iv[j] = *idum;
    }
    iy = iv[0];
  }
  k = (*idum) / IQ1;                         // Start here when not initializing
  *idum = IA1 * (*idum - k * IQ1) - k * IR1; // Compute idum=(IA1*idum) % IMR1 without
  if (*idum < 0) *idum += IMR1;              // overflows by Schrage's method
  k = idum2 / IQ2;
  idum2 = IA2 * (idum2 - k * IQ2) - k * IR2; // Compute idum2=(IA2*idum) % IMR2 likewise
  if (idum2 < 0) idum2 += IMR2;
  j = static_cast<int>(iy / NDIV); // Will be in the range 0...NTAB-1
  iy = iv[j] - idum2;              // Here idum is shuffled, idum and idum2
  iv[j] = *idum;                   // are combined to generate output
  if (iy < 1) iy += IMM1;

  if ((temp = AM * iy) > RNMX)
    return RNMX; // No endpoint values
  else
    return temp;
}

} // namespace parthenon

#undef IMR1
#undef IMR2
#undef AM
#undef IMM1
#undef IA1
#undef IA2
#undef IQ1
#undef IQ2
#undef IR1
#undef IR2
#undef NTAB
#undef NDIV
#undef RNMX
