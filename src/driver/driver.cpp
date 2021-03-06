
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

#include <algorithm>
#include <iomanip>
#include <limits>

#include "driver/driver.hpp"

#include "mesh/mesh.hpp"
#include "outputs/outputs.hpp"
#include "parameter_input.hpp"
#include "parthenon_mpi.hpp"
#include "utils/utils.hpp"

namespace parthenon {

DriverStatus EvolutionDriver::Execute() {
  InitializeBlockTimeSteps();
  SetGlobalTimeStep();
  pouts->MakeOutputs(pmesh, pinput, &tm);
  pmesh->mbcnt = 0;
  while (tm.KeepGoing()) {
    if (Globals::my_rank == 0) OutputCycleDiagnostics();

    TaskListStatus status = Step();
    if (status != TaskListStatus::complete) {
      std::cerr << "Step failed to complete all tasks." << std::endl;
      return DriverStatus::failed;
    }
    // pmesh->UserWorkInLoop();

    tm.ncycle++;
    tm.time += tm.dt;
    pmesh->mbcnt += pmesh->nbtotal;
    pmesh->step_since_lb++;

    pmesh->LoadBalancingAndAdaptiveMeshRefinement(pinput);
    if (pmesh->modified) InitializeBlockTimeSteps();
    SetGlobalTimeStep();
    if (tm.time < tm.tlim) // skip the final output as it happens later
      pouts->MakeOutputs(pmesh, pinput, &tm);

    // check for signals
    if (SignalHandler::CheckSignalFlags() != 0) {
      return DriverStatus::failed;
    }
  } // END OF MAIN INTEGRATION LOOP ======================================================

  pmesh->UserWorkAfterLoop(pinput, tm);

  DriverStatus status = DriverStatus::complete;

  pouts->MakeOutputs(pmesh, pinput, &tm);
  Report(status);

  return status;
}

void EvolutionDriver::InitializeBlockTimeSteps() {
  // calculate the first time step
  MeshBlock *pmb = pmesh->pblock;
  while (pmb != nullptr) {
    pmb->SetBlockTimestep(Update::EstimateTimestep(pmb->real_containers.Get()));
    pmb = pmb->next;
  }
}

void EvolutionDriver::Report(DriverStatus status) {
  if (Globals::my_rank == 0) SignalHandler::CancelWallTimeAlarm();

  // Print diagnostic messages related to the end of the simulation
  if (Globals::my_rank == 0) {
    OutputCycleDiagnostics();
    SignalHandler::Report();
    if (status == DriverStatus::complete) {
      std::cout << std::endl << "Driver completed." << std::endl;
    } else if (status == DriverStatus::timeout) {
      std::cout << std::endl << "Driver timed out.  Restart to continue." << std::endl;
    } else if (status == DriverStatus::failed) {
      std::cout << std::endl << "Driver failed." << std::endl;
    }

    std::cout << "time=" << tm.time << " cycle=" << tm.ncycle << std::endl;
    std::cout << "tlim=" << tm.tlim << " nlim=" << tm.nlim << std::endl;

    if (pmesh->adaptive) {
      std::cout << std::endl
                << "Number of MeshBlocks = " << pmesh->nbtotal << "; " << pmesh->nbnew
                << "  created, " << pmesh->nbdel << " destroyed during this simulation."
                << std::endl;
    }
  }
}

//----------------------------------------------------------------------------------------
// \!fn void EvolutionDriver::SetGlobalTimeStep()
// \brief function that loops over all MeshBlocks and find new timestep

void EvolutionDriver::SetGlobalTimeStep() {
  MeshBlock *pmb = pmesh->pblock;

  Real dt_max = 2.0 * tm.dt;
  tm.dt = std::numeric_limits<Real>::max();
  while (pmb != nullptr) {
    tm.dt = std::min(tm.dt, pmb->NewDt());
    pmb = pmb->next;
  }
  tm.dt = std::min(dt_max, tm.dt);

#ifdef MPI_PARALLEL
  MPI_Allreduce(MPI_IN_PLACE, &tm.dt, 1, MPI_ATHENA_REAL, MPI_MIN, MPI_COMM_WORLD);
#endif

  if (tm.time < tm.tlim &&
      (tm.tlim - tm.time) < tm.dt) // timestep would take us past desired endpoint
    tm.dt = tm.tlim - tm.time;

  return;
}

void EvolutionDriver::OutputCycleDiagnostics() {
  const int dt_precision = std::numeric_limits<Real>::max_digits10 - 1;
  const int ratio_precision = 3;
  if (tm.ncycle_out != 0) {
    if (tm.ncycle % tm.ncycle_out == 0) {
      if (Globals::my_rank == 0) {
        std::cout << "cycle=" << tm.ncycle << std::scientific
                  << std::setprecision(dt_precision) << " time=" << tm.time
                  << " dt=" << tm.dt;
        // insert more diagnostics here
        std::cout << std::endl;
      }
    }
  }
  return;
}

} // namespace parthenon
