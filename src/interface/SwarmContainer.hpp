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
#ifndef INTERFACE_SWARM_CONTAINER_HPP_
#define INTERFACE_SWARM_CONTAINER_HPP_

#include <map>
#include <memory>
#include <string>
#include <utility> // <pair>
#include <vector>
#include "globals.hpp"
//#include "mesh/mesh.hpp"
#include "Swarm.hpp"

namespace parthenon {
///
/// Interface to underlying infrastructure for particle data declaration and
/// access.
/// Date: August 22, 2019
///
///
/// The SwarmContainer class is a container for the swarms of particles that
/// make up the simulation.
///
/// The container class will provide the following methods:
///

class MeshBlock;

class SwarmContainer {
 public:
  //-----------------
  // Public Variables
  //-----------------
  MeshBlock* pmy_block = nullptr; // ptr to MeshBlock

  //-----------------
  //Public Methods
  //-----------------
  // Constructor does nothing
  SwarmContainer() {
  }

  ///
  /// Set the pointer to the mesh block for this container
  void setBlock(MeshBlock *pmb) { pmy_block = pmb; }

  ///
  /// Allocate and add a variable<T> to the container
  ///
  /// This function will eventually look at the metadata flags to
  /// identify the size of the first dimension based on the
  /// topological location.  Dimensions will be taken from the metadata.
  ///
  /// @param label the name of the variable
  /// @param metadata the metadata associated with the variable
  ///
  void Add(const std::string label, const SwarmMetadata &smetadata);

  ///
  /// Allocate and add a variable<T> to the container
  ///
  /// This function will eventually look at the metadata flags to
  /// identify the size of the first dimension based on the
  /// topological location.  Dimensions will be taken from the metadata.
  ///
  /// @param labelArray the array of names of variables
  /// @param metadata the metadata associated with the variable
  ///
  void Add(const std::vector<std::string> labelArray, const SwarmMetadata &smetadata);

  ///
  /// Get a swarm from the container
  /// @param label the name of the swarm
  /// @return the Swarm if found or throw exception
  Swarm& Get(std::string label) {
    for (auto v : swarms) {
      if (! v->label().compare(label)) return *v;
    }
    throw std::invalid_argument (std::string("\n") +
                                 std::string(label) +
                                 std::string(" swarm not found in Get()\n") );
  }

  Swarm& Get(const int index) {
    return *(swarms[index]);
  }

  int Index(const std::string& label) {
    for (int i = 0; i < swarms.size(); i++) {
      if (! swarms[i]->label().compare(label)) return i;
    }
    return -1;
  }

  /// Gets an array of real variables from container.
  /// @param names is the variables we want
  /// @param indexCount a map of names to std::pair<index,count> for each name
  /// @param sparse_ids if specified is list of sparse ids we are interested in.  Note
  ///        that non-sparse variables specified are aliased in as is.
  int GetSwarms(const std::vector<std::string>& names,
                std::vector<Swarm>& sRet,
                std::map<std::string,std::pair<int,int>>& indexCount);

  ///
  /// Remove a variable from the container or throw exception if not
  /// found.
  /// @param label the name of the variable to be deleted
  void Remove(const std::string label);

  // Temporary functions till we implement a *real* iterator

  /// Print list of labels in container
  void print();

  // return number of stored arrays
  int size() {return swarms.size();}

  // Element accessor functions
  std::vector<std::shared_ptr<Swarm>>& allSwarms() {
    return swarms;
  }

  // Communication routines
  void SetupPersistentMPI();
  void SetBoundaries();
  void SendBoundaryBuffers();
  void ReceiveAndSetBoundariesWithWait();
  bool ReceiveBoundaryBuffers();
  void StartReceiving(BoundaryCommSubset phase);
  void ClearBoundary(BoundaryCommSubset phase);

 int GetVariables(const std::vector<std::string>& names,
                               std::vector<Swarm>& sRet,
                               std::map<std::string,std::pair<int,int>>& indexCount,
                               const std::vector<int>& sparse_ids);

 private:
  int debug=0;
  std::vector<std::shared_ptr<Swarm>> swarms = {};
};

} // namespace parthenon
#endif // INTERFACE_SWARM_CONTAINER_HPP_