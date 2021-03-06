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
//! \file change_rundir.cpp
//! \brief executes unix 'chdir' command to change dir in which Athena++ runs

// POSIX C extensions
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <stdexcept>

#include "athena.hpp"

namespace parthenon {

//----------------------------------------------------------------------------------------
//! \fn void ChangeRunDir(const char *pdir)
//  \brief change to input run directory; create if it does not exist yet

void ChangeRunDir(const char *pdir) {
  std::stringstream msg;

  if (pdir == nullptr || *pdir == '\0') return;

  mkdir(pdir, 0775);
  if (chdir(pdir)) {
    msg << "### FATAL ERROR in function [ChangeToRunDir]" << std::endl
        << "Cannot cd to directory '" << pdir << "'";
    ATHENA_ERROR(msg);
  }

  return;
}

} // namespace parthenon
