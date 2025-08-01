/*  This file is part of the OpenLB library
 *
 *  Copyright (C) 2014 Peter Weisbrod
 *  E-mail contact: info@openlb.net
 *  The most recent release of OpenLB can be downloaded at
 *  <http://www.openlb.net/>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this program; if not, write to the Free
 *  Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
*/

/* phaseSeparation2d.cpp:
 * In this example the simulation is initialized with a given
 * density plus a small random number all over the domain. This
 * condition is unstable and leads to liquid-vapor phase separation.
 * Boundaries are assumed to be periodic. This example shows the
 * usage of multiphase flow.
 */


#include <olb.h>

using namespace olb;
using namespace olb::descriptors;
using namespace olb::graphics;

using T = FLOATING_POINT_TYPE;
using DESCRIPTOR = D2Q9<VELOCITY,FORCE,EXTERNAL_FORCE,STATISTIC>;
using BulkDynamics = ForcedShanChenBGKdynamics<T, DESCRIPTOR, momenta::ExternalVelocityTuple>;


// Parameters for the simulation setup
const int maxIter  = 10000;
const int nx   = 201;
const int ny   = 201;


// Stores geometry information in form of material numbers
void prepareGeometry( SuperGeometry<T,2>& superGeometry )
{

  OstreamManager clout( std::cout,"prepareGeometry" );
  clout << "Prepare Geometry ..." << std::endl;

  // Sets material number for fluid
  superGeometry.rename( 0,1 );

  // Removes all not needed boundary voxels outside the surface
  superGeometry.clean();
  // Removes all not needed boundary voxels inside the surface
  superGeometry.innerClean();
  superGeometry.checkForErrors();

  superGeometry.print();

  clout << "Prepare Geometry ... OK" << std::endl;
}

// Set up the geometry of the simulation
void prepareLattice( SuperLattice<T, DESCRIPTOR>& sLattice,
                     SuperGeometry<T,2>& superGeometry )
{
  const T omega1 = 1.0;

  // Material=1 -->bulk dynamics
  sLattice.defineDynamics<BulkDynamics>(superGeometry, 1);

  // Initial conditions
  AnalyticalConst2D<T,T> noise( 2. );
  std::vector<T> v( 2,T() );
  AnalyticalConst2D<T,T> zeroVelocity( v );
  AnalyticalConst2D<T,T> oldRho( 199. );
  AnalyticalRandom2D<T,T> random;
  AnalyticalIdentity2D<T,T> newRho( random*noise+oldRho );

  // Initialize all values of distribution functions to their local equilibrium
  sLattice.defineRhoU( superGeometry, 1, newRho, zeroVelocity );
  sLattice.iniEquilibrium( superGeometry, 1, newRho, zeroVelocity );

  sLattice.setParameter<descriptors::OMEGA>(omega1);

  sLattice.addPostProcessor<stage::PreCoupling>(meta::id<RhoStatistics>());
  {
    auto& communicator = sLattice.getCommunicator(stage::Coupling());
    communicator.requestField<descriptors::STATISTIC>();
    communicator.requestOverlap(1);
    communicator.exchangeRequests();
  }

  using COUPLING = ShanChenForcedSingleComponentPostProcessor<T,DESCRIPTOR,interaction::ShanChen94>;
  sLattice.addPostProcessor<stage::Coupling>(meta::id<COUPLING>{});
  sLattice.setParameter<COUPLING::G>(T(-120));
  sLattice.setParameter<COUPLING::RHO0>(T(1));
  sLattice.setParameter<COUPLING::OMEGA>(omega1);

  sLattice.template addCustomTask<stage::PostStream>([&]() {
    sLattice.executePostProcessors(stage::PreCoupling());
    sLattice.executePostProcessors(stage::Coupling());
  });

  // Make the lattice ready for simulation
  sLattice.initialize();
}

// Output to console and files
void getResults( SuperLattice<T, DESCRIPTOR>& sLattice, int iT,
                 SuperGeometry<T,2>& superGeometry, util::Timer<T>& timer )
{

  OstreamManager clout( std::cout,"getResults" );

  SuperVTMwriter2D<T> vtmWriter( "phaseSeparation2d" );
  SuperLatticeVelocity2D<T, DESCRIPTOR> velocity( sLattice );
  SuperLatticeDensity2D<T, DESCRIPTOR> density( sLattice );
  vtmWriter.addFunctor( velocity );
  vtmWriter.addFunctor( density );

  const int vtkIter  = 20;
  const int statIter = 20;

  if ( iT==0 ) {
    // Writes the geometry, cuboid no. and rank no. as vti file for visualization
    SuperLatticeCuboid2D<T, DESCRIPTOR> cuboid( sLattice );
    SuperLatticeRank2D<T, DESCRIPTOR> rank( sLattice );
    vtmWriter.write( cuboid );
    vtmWriter.write( rank );

    vtmWriter.createMasterFile();
  }

  // Writes the vtk files
  if ( iT%vtkIter==0 ) {
    sLattice.setProcessingContext(ProcessingContext::Evaluation);

    clout << "Writing VTK and JPEG..." << std::endl;
    vtmWriter.write( iT );

    BlockReduction2D2D<T> planeReduction( density, 600, BlockDataSyncMode::ReduceOnly );
    // write output as JPEG
    heatmap::write(planeReduction, iT);
  }

  // Writes output on the console
  if ( iT%statIter==0 ) {
    // Timer console output
    timer.update( iT );
    timer.printStep();

    // Lattice statistics console output
    sLattice.getStatistics().print( iT,iT );
  }
}

int main( int argc, char *argv[] )
{
  // === 1st Step: Initialization ===
  initialize( &argc, &argv );
  singleton::directories().setOutputDir( "./tmp/" );
  OstreamManager clout( std::cout,"main" );
  // display messages from every single mpi process
  //clout.setMultiOutput(true);

  // === 2rd Step: Prepare Geometry ===

  // Instantiation of a cuboidDecomposition with weights
#ifdef PARALLEL_MODE_MPI
  const int noOfCuboids = singleton::mpi().getSize();
#else
  const int noOfCuboids = 1;
#endif
  CuboidDecomposition2D<T> cuboidDecomposition(0, 1, {nx, ny}, noOfCuboids);

  // Periodic boundaries in x- and y-direction
  cuboidDecomposition.setPeriodicity({ true, true });

  // Instantiation of a loadBalancer
  HeuristicLoadBalancer<T> loadBalancer( cuboidDecomposition );

  // Instantiation of a superGeometry
  SuperGeometry<T,2> superGeometry( cuboidDecomposition,loadBalancer,2 );

  prepareGeometry( superGeometry );

  // === 3rd Step: Prepare Lattice ===
  SuperLattice<T, DESCRIPTOR> sLattice( superGeometry );

  prepareLattice( sLattice, superGeometry );

  // === 4th Step: Main Loop ===
  int iT = 0;
  clout << "starting simulation..." << std::endl;
  util::Timer<T> timer( maxIter, superGeometry.getStatistics().getNvoxel() );
  timer.start();

  for ( iT = 0; iT < maxIter; ++iT ) {

    // === 5th Step: Definition of Initial and Boundary Conditions ===
    // in this application no boundary conditions have to be adjusted

    // === 6th Step: Collide and Stream Execution ===
    sLattice.collideAndStream();

    // === 7th Step: Computation and Output of the Results ===
    getResults( sLattice, iT, superGeometry, timer );
  }

  timer.stop();
  timer.printSummary();
}
