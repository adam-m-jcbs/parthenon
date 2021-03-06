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
#ifndef EXAMPLE_CALCULATE_PI_CALCULATE_PI_HPP_
#define EXAMPLE_CALCULATE_PI_CALCULATE_PI_HPP_

// Standard Includes
#include <memory>

// Parthenon Includes
#include <parthenon/package.hpp>

namespace calculate_pi {
using namespace parthenon::package::prelude;

// Package Callbacks
void SetInOrOut(Container<Real> &rc);
parthenon::AmrTag CheckRefinement(Container<Real> &rc);
std::shared_ptr<StateDescriptor> Initialize(ParameterInput *pin);

// Task Implementations
parthenon::TaskStatus ComputeArea(parthenon::MeshBlock *pmb);
} // namespace calculate_pi

#endif // EXAMPLE_CALCULATE_PI_CALCULATE_PI_HPP_
