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

#include "interface/update.hpp"

#include <algorithm>
#include <limits>

#include "coordinates/coordinates.hpp"
#include "interface/container.hpp"
#include "mesh/mesh.hpp"

#include "kokkos_abstraction.hpp"

namespace parthenon {

namespace Update {

TaskStatus FluxDivergence(Container<Real> &in, Container<Real> &dudt_cont) {
  MeshBlock *pmb = in.pmy_block;
  int is = pmb->is;
  int js = pmb->js;
  int ks = pmb->ks;
  int ie = pmb->ie;
  int je = pmb->je;
  int ke = pmb->ke;

  auto vin = in.PackVariablesAndFluxes({Metadata::Independent});
  auto dudt = dudt_cont.PackVariables({Metadata::Independent});

  auto &coords = pmb->coords;
  int ndim = pmb->pmy_mesh->ndim;
  pmb->par_for(
      "flux divergence", 0, vin.GetDim(4) - 1, ks, ke, js, je, is, ie,
      KOKKOS_LAMBDA(const int l, const int k, const int j, const int i) {
        dudt(l, k, j, i) = 0.0;
        dudt(l, k, j, i) +=
            (coords.Area(X1DIR, k, j, i + 1) * vin.flux(X1DIR, l, k, j, i + 1) -
             coords.Area(X1DIR, k, j, i) * vin.flux(X1DIR, l, k, j, i));
        if (ndim >= 2) {
          dudt(l, k, j, i) +=
              (coords.Area(X2DIR, k, j + 1, i) * vin.flux(X2DIR, l, k, j + 1, i) -
               coords.Area(X2DIR, k, j, i) * vin.flux(X2DIR, l, k, j, i));
        }
        if (ndim == 3) {
          dudt(l, k, j, i) +=
              (coords.Area(X3DIR, k + 1, j, i) * vin.flux(X3DIR, l, k + 1, j, i) -
               coords.Area(X3DIR, k, j, i) * vin.flux(X3DIR, l, k, j, i));
        }
        dudt(l, k, j, i) /= -coords.Volume(k, j, i);
      });

  return TaskStatus::complete;
}

void UpdateContainer(Container<Real> &in, Container<Real> &dudt_cont, const Real dt,
                     Container<Real> &out) {
  MeshBlock *pmb = in.pmy_block;
  int is = pmb->is;
  int js = pmb->js;
  int ks = pmb->ks;
  int ie = pmb->ie;
  int je = pmb->je;
  int ke = pmb->ke;

  auto vin = in.PackVariables({Metadata::Independent});
  auto vout = out.PackVariables({Metadata::Independent});
  auto dudt = dudt_cont.PackVariables({Metadata::Independent});

  pmb->par_for(
      "UpdateContainer", 0, vin.GetDim(4) - 1, 0, vin.GetDim(3) - 1, 0, vin.GetDim(2) - 1,
      0, vin.GetDim(1) - 1,
      KOKKOS_LAMBDA(const int l, const int k, const int j, const int i) {
        vout(l, k, j, i) = vin(l, k, j, i) + dt * dudt(l, k, j, i);
      });
  return;
}

void AverageContainers(Container<Real> &c1, Container<Real> &c2, const Real wgt1) {
  MeshBlock *pmb = c1.pmy_block;
  int is = pmb->is;
  int js = pmb->js;
  int ks = pmb->ks;
  int ie = pmb->ie;
  int je = pmb->je;
  int ke = pmb->ke;

  auto v1 = c1.PackVariables({Metadata::Independent});
  auto v2 = c2.PackVariables({Metadata::Independent});

  pmb->par_for(
      "AverageContainers", 0, v1.GetDim(4) - 1, ks, ke, js, je, is, ie,
      KOKKOS_LAMBDA(const int l, const int k, const int j, const int i) {
        v1(l, k, j, i) = wgt1 * v1(l, k, j, i) + (1 - wgt1) * v2(l, k, j, i);
      });

  return;
}

Real EstimateTimestep(Container<Real> &rc) {
  MeshBlock *pmb = rc.pmy_block;
  Real dt_min = std::numeric_limits<Real>::max();
  for (auto &pkg : pmb->packages) {
    auto &desc = pkg.second;
    if (desc->EstimateTimestep != nullptr) {
      Real dt = desc->EstimateTimestep(rc);
      dt_min = std::min(dt_min, dt);
    }
  }
  return dt_min;
}

} // namespace Update

static FillDerivedVariables::FillDerivedFunc *pre_package_fill_ = nullptr;
static FillDerivedVariables::FillDerivedFunc *post_package_fill_ = nullptr;

void FillDerivedVariables::SetFillDerivedFunctions(FillDerivedFunc *pre,
                                                   FillDerivedFunc *post) {
  pre_package_fill_ = pre;
  post_package_fill_ = post;
}

TaskStatus FillDerivedVariables::FillDerived(Container<Real> &rc) {
  if (pre_package_fill_ != nullptr) {
    pre_package_fill_(rc);
  }
  for (auto &pkg : rc.pmy_block->packages) {
    auto &desc = pkg.second;
    if (desc->FillDerived != nullptr) {
      desc->FillDerived(rc);
    }
  }
  if (post_package_fill_ != nullptr) {
    post_package_fill_(rc);
  }
  return TaskStatus::complete;
}

} // namespace parthenon
