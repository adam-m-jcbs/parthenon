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

#ifndef PARTHENON_MANAGER_HPP_
#define PARTHENON_MANAGER_HPP_

#include <memory>

#include "argument_parser.hpp"
#include "basic_types.hpp"
#include "driver/driver.hpp"
#include "interface/properties_interface.hpp"
#include "interface/state_descriptor.hpp"
#include "mesh/mesh.hpp"
#include "outputs/outputs.hpp"
#include "parameter_input.hpp"

namespace parthenon {

enum class ParthenonStatus { ok, complete, error };

class ParthenonManager {
 public:
  ParthenonManager() = default;
  ParthenonStatus ParthenonInit(int argc, char *argv[]);
  ParthenonStatus ParthenonFinalize();

  bool Restart() { return (arg.restart_filename == nullptr ? false : true); }
  Properties_t ProcessProperties(std::unique_ptr<ParameterInput> &pin);
  Packages_t ProcessPackages(std::unique_ptr<ParameterInput> &pin);
  void SetFillDerivedFunctions();
  void PreDriver();
  void PostDriver(DriverStatus driver_status);

  // member data
  std::unique_ptr<ParameterInput> pinput;
  std::unique_ptr<Mesh> pmesh;

 private:
  ArgParse arg;
  clock_t tstart_;
#ifdef OPENMP_PARALLEL
  double omp_start_time_;
#endif
};

} // namespace parthenon

#endif // PARTHENON_MANAGER_HPP_
