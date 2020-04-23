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
#ifndef INTERFACE_UPDATE_HPP_
#define INTERFACE_UPDATE_HPP_

#include "athena.hpp"
#include "interface/container.hpp"
#include "mesh/mesh.hpp"

namespace parthenon {

namespace Update {

TaskStatus FluxDivergence(Container<Real> &in, Container<Real> &dudt_cont);
void UpdateContainer(Container<Real> &in, Container<Real> &dudt_cont, const Real dt,
                     Container<Real> &out);
TaskStatus TransportSwarm(Swarm &in, Swarm &out);
void AverageContainers(Container<Real> &c1, Container<Real> &c2, const Real wgt1);
Real EstimateTimestep(Container<Real> &rc);

} // namespace Update

namespace FillDerivedVariables {

using FillDerivedFunc = void(Container<Real> &);
void SetFillDerivedFunctions(FillDerivedFunc *pre, FillDerivedFunc *post);
TaskStatus FillDerived(Container<Real> &rc);

} // namespace FillDerivedVariables

} // namespace parthenon

#endif // INTERFACE_UPDATE_HPP_