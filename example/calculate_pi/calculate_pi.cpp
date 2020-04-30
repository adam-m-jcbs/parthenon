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

// Self Include
#include "calculate_pi.hpp"

// Standard Includes
#include <iostream>
#include <string>
#include <utility>

// Parthenon Includes
#include <parthenon/package.hpp>

using namespace parthenon::package::prelude;

using parthenon::Coordinates;

namespace parthenon {

// can be used to set global properties that all meshblocks want to know about
// no need in this app so use the weak version that ships with parthenon
// Properties_t ParthenonManager::ProcessProperties(std::unique_ptr<ParameterInput>& pin)
// {
//  Properties_t props;
//  return props;
//}

Packages_t ParthenonManager::ProcessPackages(std::unique_ptr<ParameterInput> &pin) {
  Packages_t packages;
  // only have one package for this app, but will typically have more things added to
  packages["calculate_pi"] = calculate_pi::Initialize(pin.get());
  return packages;
}

// this should set up initial conditions of independent variables on the block
// this app only has one variable of derived type, so nothing to do here.
// in this case, just use the weak version
// void MeshBlock::ProblemGenerator(ParameterInput *pin) {
//  // nothing to do here for this app
//}

// applications can register functions to fill shared derived quantities
// before and/or after all the package FillDerived call backs
// in this case, just use the weak version that sets these to nullptr
// void ParthenonManager::SetFillDerivedFunctions() {
//  FillDerivedVariables::SetFillDerivedFunctions(nullptr,nullptr);
//}

} // namespace parthenon

// This defines a "physics" package
// In this case, calculate_pi provides the functions required to set up
// an indicator function in_or_out(x,y) = (r < r0 ? 1 : 0), and compute the area
// of a circle of radius r0 as A = \int d^x in_or_out(x,y) over the domain. Then
// pi \approx A/r0^2
namespace calculate_pi {

void SetInOrOut(Container<Real> &rc) {
  MeshBlock *pmb = rc.pmy_block;
  int is = pmb->is;
  int js = pmb->js;
  int ks = pmb->ks;
  int ie = pmb->ie;
  int je = pmb->je;
  int ke = pmb->ke;
  Coordinates *pcoord = pmb->pcoord.get();
  CellVariable<Real> &v = rc.Get("in_or_out");
  const auto &radius = pmb->packages["calculate_pi"]->Param<Real>("radius");
  // Set an indicator function that indicates whether the cell center
  // is inside or outside of the circle we're interating the area of.
  // see the CheckRefinement routine below for an explanation of the loop bounds
  for (int k = ks; k <= ke; k++) {
    for (int j = js - 1; j <= je + 1; j++) {
      for (int i = is - 1; i <= ie + 1; i++) {
        Real rsq = std::pow(pcoord->x1v(i), 2) + std::pow(pcoord->x2v(j), 2);
        if (rsq < radius * radius) {
          v(k, j, i) = 1.0;
        } else {
          v(k, j, i) = 0.0;
        }
      }
    }
  }
  /** TODO(pgrete) This is what it should should like using the transparent
    * parallel_for wrapper of the MeshBlock.
    * Unfortunely, the current Container/CellVariable/ParArrayND combination
    * won't work this way and we should discuss how to proceed before
    * starting a bigger refactoring of the the classes above.

  auto x1v = pcoord->x1v; // LAMBDA doesn't capture member vars so we need to redef.
  auto x2v = pcoord->x2v;
  pmb->par_for(
      "SetInOrOut", ks, ke, js - 1, je + 1, is - 1, ie + 1,
      KOKKOS_LAMBDA(const int k, const int j, const int i) {
        Real rsq = pow(x1v(i), 2) + pow(x2v(j), 2);
        if (rsq < radius * radius) {
          v(k, j, i) = 1.0;
        } else {
          v(k, j, i) = 0.0;
        }
      });
  */
}

AmrTag CheckRefinement(Container<Real> &rc) {
  // tag cells for refinement or derefinement
  // each package can define its own refinement tagging
  // function and they are all called by parthenon
  MeshBlock *pmb = rc.pmy_block;
  int is = pmb->is;
  int js = pmb->js;
  int ks = pmb->ks;
  int ie = pmb->ie;
  int je = pmb->je;
  int ke = pmb->ke;
  CellVariable<Real> &v = rc.Get("in_or_out");
  AmrTag delta_level = AmrTag::derefine;
  Real vmin = 1.0;
  Real vmax = 0.0;
  // loop over all real cells and one layer of ghost cells and refine
  // if the edge of the circle is found.  The one layer of ghost cells
  // catches the case where the edge is between the cell centers of
  // the first/last real cell and the first ghost cell
  for (int k = ks; k <= ke; k++) {
    for (int j = js - 1; j <= je + 1; j++) {
      for (int i = is - 1; i <= ie + 1; i++) {
        vmin = (v(k, j, i) < vmin ? v(k, j, i) : vmin);
        vmax = (v(k, j, i) > vmax ? v(k, j, i) : vmax);
      }
    }
  }
  // was the edge of the circle found?
  if (vmax > 0.95 && vmin < 0.05) { // then yes
    delta_level = AmrTag::refine;
  }
  return delta_level;
}

std::shared_ptr<StateDescriptor> Initialize(ParameterInput *pin) {
  auto package = std::make_shared<StateDescriptor>("calculate_pi");
  Params &params = package->AllParams();

  Real radius = pin->GetOrAddReal("Pi", "radius", 1.0);
  params.Add("radius", radius);

  // add a variable called in_or_out that will hold the value of the indicator function
  std::string field_name("in_or_out");
  Metadata m({Metadata::Cell, Metadata::Derived});
  package->AddField(field_name, m, DerivedOwnership::unique);

  // All the package FillDerived and CheckRefinement functions are called by parthenon
  package->FillDerived = SetInOrOut;
  // could use this package specific refinement tagging routine (above), but
  // instead this example will make use of the parthenon shipped first derivative
  // criteria, as invoked in the input file
  // package->CheckRefinement = CheckRefinement;
  return package;
}

TaskStatus ComputeArea(MeshBlock *pmb) {
  // compute 1/r0^2 \int d^2x in_or_out(x,y) over the block's domain
  Container<Real> &rc = pmb->real_containers.Get();
  int is = pmb->is;
  int js = pmb->js;
  int ks = pmb->ks;
  int ie = pmb->ie;
  int je = pmb->je;
  int ke = pmb->ke;
  Coordinates *pcoord = pmb->pcoord.get();
  CellVariable<Real> &v = rc.Get("in_or_out");
  const auto &radius = pmb->packages["calculate_pi"]->Param<Real>("radius");
  Real area = 0.0;
  for (int k = ks; k <= ke; k++) {
    for (int j = js; j <= je; j++) {
      for (int i = is; i <= ie; i++) {
        area += v(k, j, i) * pcoord->dx1f(i) * pcoord->dx2f(j);
      }
    }
  }
  // std::cout << "area = " << area << std::endl;
  area /= (radius * radius);
  // just stash the area somewhere for later
  v(0, 0, 0) = area;
  return TaskStatus::complete;
}

} // namespace calculate_pi
