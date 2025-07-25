/*  Lattice Boltzmann sample, written in C++, using the OpenLB
 *  library
 *
 *  Copyright (C) 2019 Sam Avis
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

/*  microfluidics2d.cpp:
 *  This example shows a microfluidic channel creating droplets of
 *  two fluid components. Poiseuille velocity profiles are imposed
 *  at the various channel inlets, while a constant density outlet
 *  is imposed at the end of the channel to allow the droplets to
 *  exit the simulation.
 *
 *  This example demonstrates the use of three fluid components
 *  with the free energy model. It also shows the use of open
 *  boundary conditions, specifically velocity inlet and density
 *  outlet boundaries.
 */

#include <olb.h>

using namespace olb;
using namespace olb::descriptors;
using namespace olb::graphics;

using T = FLOATING_POINT_TYPE;
typedef D2Q9<CHEM_POTENTIAL,FORCE> DESCRIPTOR;

// Parameters for the simulation setup
const int N  = 50;
const T nx   = 800.;
const T ny   = 100.;
const T dx = ny / N;

const T inSize = 175.;
const T xl1 = inSize * 2./7.;
const T yl1 = ny / 4.;
const T xl2 = inSize / 7.;
const T yl2 = ny;
const T xl3 = inSize * 3./7.;
const T yl3 = ny / 4.;
const T xl4 = inSize / 7.;
const T yl4 = ny;
const T xl5 = nx - inSize;
const T yl5 = ny / 2.;

const T inlet1Velocity = 0.00056; // [lattice units]
const T inlet2Velocity = 0.00055; // [lattice units]
const T inlet3Velocity = 0.0014;  // [lattice units]
const T outletDensity = 1.;       // [lattice units]
const T alpha = 1.;               // Interfacial width          [lattice units]
const T kappa1 = 0.0132;          // For surface tensions       [lattice units]
const T kappa2 = 0.0012;          // For surface tensions       [lattice units]
const T kappa3 = 0.0013;          // For surface tensions       [lattice units]
const T gama = 1.;                // For mobility of interfaces [lattice units]
const T h1 = 0.;                  // Contact angle 90 degrees   [lattice units]
const T h2 = 0.;                  // Contact angle 90 degrees   [lattice units]
const T h3 = 0.;                  // Contact angle 90 degrees   [lattice units]

const int maxIter  = 1000000;
const int vtkIter  = 1000;
const int statIter = 2000;

T helperFunction( T alpha, T kappa1, T kappa2, T h1, T h2, int latticeNumber )
{
  T addend = 0;
  if (latticeNumber==1) {
    addend = 1./(alpha*alpha) * ( (h1/kappa1) + (h2/kappa2) );
  }
  else if (latticeNumber==2) {
    addend = 1./(alpha*alpha) * ( (h1/kappa1) + (-h2/kappa2) );
  }
  else if (latticeNumber==3) {
    addend = 1./(alpha*alpha) * ( (h1/kappa1) + (h2/kappa2) );
  }
  return addend;
}

T helperFunction( T alpha, T kappa1, T kappa2, T kappa3, T h1, T h2, T h3, int latticeNumber )
{
  T addend = 0;
  if (latticeNumber==1) {
    addend = 1./(alpha*alpha) * ( (h1/kappa1) + (h2/kappa2) + (h3/kappa3) );
  }
  else if (latticeNumber==2) {
    addend = 1./(alpha*alpha) * ( (h1/kappa1) + (-h2/kappa2) );
  }
  else if (latticeNumber==3) {
    addend = 1./(alpha*alpha) * ( (h3/kappa3) );
  }
  return addend;
}

void prepareGeometry( SuperGeometry<T,2>& superGeometry )
{
  OstreamManager clout( std::cout,"prepareGeometry" );
  clout << "Prepare Geometry ..." << std::endl;

  std::shared_ptr<IndicatorF2D<T>> section1 = std::make_shared<IndicatorCuboid2D<T>>( xl1, yl1, std::vector<T> {xl1/T(2), ny/T(2)} );
  std::shared_ptr<IndicatorF2D<T>> section2 = std::make_shared<IndicatorCuboid2D<T>>( xl2, yl2, std::vector<T> {xl1 + xl2/T(2), ny/T(2)} );
  std::shared_ptr<IndicatorF2D<T>> section3 = std::make_shared<IndicatorCuboid2D<T>>( xl3, yl3, std::vector<T> {xl1 + xl2 + xl3/T(2), ny/T(2)} );
  std::shared_ptr<IndicatorF2D<T>> section4 = std::make_shared<IndicatorCuboid2D<T>>( xl4, yl4, std::vector<T> {xl1 + xl2 + xl3 + xl4/T(2), ny/T(2)} );
  std::shared_ptr<IndicatorF2D<T>> section5 = std::make_shared<IndicatorCuboid2D<T>>( xl5, yl5, std::vector<T> {xl1 + xl2 + xl3 + xl4 + xl5/T(2), ny/T(2)} );
  IndicatorIdentity2D<T> channel( section1 + section2 + section3 + section4 + section5 );

  superGeometry.rename( 0, 2, channel );
  superGeometry.rename( 2,1,{1,1} );

  // Inlets and outlet
  IndicatorCuboid2D<T> inlet1 ( dx, yl1, {0., ny/T(2)} );
  IndicatorCuboid2D<T> inlet21( xl2 - dx, dx, {xl1 + xl2/T(2), 0.} );
  IndicatorCuboid2D<T> inlet22( xl2 - dx, dx, {xl1 + xl2/T(2), ny} );
  IndicatorCuboid2D<T> inlet31( xl4 - dx, dx, {xl1 + xl2 + xl3 + xl4/T(2), 0.} );
  IndicatorCuboid2D<T> inlet32( xl4 - dx, dx, {xl1 + xl2 + xl3 + xl4/T(2), ny} );
  IndicatorCuboid2D<T> outlet( dx, yl5, {nx, ny/T(2)} );
  superGeometry.rename( 2, 3, 1, inlet1 );
  superGeometry.rename( 2, 4, 1, inlet21 );
  superGeometry.rename( 2, 5, 1, inlet22 );
  superGeometry.rename( 2, 6, 1, inlet31 );
  superGeometry.rename( 2, 7, 1, inlet32 );
  superGeometry.rename( 2, 8, 1, outlet );

  superGeometry.innerClean();
  superGeometry.checkForErrors();
  superGeometry.print();

  clout << "Prepare Geometry ... OK" << std::endl;
}


void prepareLattice( SuperLattice<T, DESCRIPTOR>& sLattice1,
                     SuperLattice<T, DESCRIPTOR>& sLattice2,
                     SuperLattice<T, DESCRIPTOR>& sLattice3,
                     UnitConverter<T, DESCRIPTOR>& converter,
                     SuperGeometry<T,2>& superGeometry )
{

  OstreamManager clout( std::cout,"prepareLattice" );
  clout << "Prepare Lattice ..." << std::endl;

  // define lattice dynamics
  clout << "Prepare Lattice: Define lattice dynamics ..." << std::endl;
  sLattice1.defineDynamics<ForcedBGKdynamics>(superGeometry, 1);
  sLattice2.defineDynamics<FreeEnergyBGKdynamics>(superGeometry, 1);
  sLattice3.defineDynamics<FreeEnergyBGKdynamics>(superGeometry, 1);

  // Defining walls
  auto walls = superGeometry.getMaterialIndicator({2});

  // Compute Addends
  T addend1 = helperFunction( alpha, kappa1, kappa2, kappa3, h1, h2, h3, 1 );
  T addend2 = helperFunction( alpha, kappa1, kappa2, kappa3, h1, h2, h3, 2 );
  T addend3 = helperFunction( alpha, kappa1, kappa2, kappa3, h1, h2, h3, 3 );

  // Add wall boundary
  clout << "Prepare Lattice: Add wall boundary ..." << std::endl;
  boundary::set<boundary::FreeEnergyWallMomentum>(sLattice1, walls);
  sLattice1.setParameter<descriptors::ADDEND>( addend1 );
  boundary::set<boundary::FreeEnergyWallOrderParameter>(sLattice2, walls);
  sLattice2.setParameter<descriptors::ADDEND>( addend2 );
  boundary::set<boundary::FreeEnergyWallOrderParameter>(sLattice3, walls);
  sLattice3.setParameter<descriptors::ADDEND>( addend3 );

  // add inlet boundaries
  clout << "Prepare Lattice: Add inlet boundaries ..." << std::endl;
  T omega = converter.getLatticeRelaxationFrequency();
  auto inlet1Indicator = superGeometry.getMaterialIndicator(3);
  boundary::set<boundary::FreeEnergyVelocity>(sLattice1, inlet1Indicator);
  boundary::set<boundary::FreeEnergyOrderParameter>(sLattice2, inlet1Indicator);
  boundary::set<boundary::FreeEnergyOrderParameter>(sLattice3, inlet1Indicator);

  auto inlet2Indicator = superGeometry.getMaterialIndicator({4, 5});
  boundary::set<boundary::FreeEnergyVelocity>(sLattice1, inlet2Indicator);
  boundary::set<boundary::FreeEnergyOrderParameter>(sLattice2, inlet2Indicator);
  boundary::set<boundary::FreeEnergyOrderParameter>(sLattice3, inlet2Indicator);

  auto inlet3Indicator = superGeometry.getMaterialIndicator({6, 7});
  boundary::set<boundary::FreeEnergyVelocity>(sLattice1, inlet3Indicator);
  boundary::set<boundary::FreeEnergyOrderParameter>(sLattice2, inlet3Indicator);
  boundary::set<boundary::FreeEnergyOrderParameter>(sLattice3, inlet3Indicator);

  // add outlet boundary
  clout << "Prepare Lattice: Add outlet boundary ..." << std::endl;
  auto outletIndicator = superGeometry.getMaterialIndicator(8);
  boundary::set<boundary::FreeEnergyPressureConvective>(sLattice1, outletIndicator);
  boundary::set<boundary::FreeEnergyOrderParameterConvective>(sLattice2, outletIndicator);
  boundary::set<boundary::FreeEnergyOrderParameterConvective>(sLattice3, outletIndicator);

  // bulk initial conditions
  clout << "Prepare Lattice: Bulk initial conditions ..." << std::endl;
  std::vector<T> v( 2,T() );
  AnalyticalConst2D<T,T> zeroVelocity( v );

  AnalyticalConst2D<T,T> zero ( 0. );
  AnalyticalConst2D<T,T> one ( 1. );
  IndicatorCuboid2D<T> ind1(xl1+dx, ny, {xl1/T(2), ny/T(2)});
  SmoothIndicatorCuboid2D<T,T> section1( ind1, 0. );
  IndicatorCuboid2D<T> ind2(xl2 + xl3, ny, {xl1 + (xl2 + xl3)/T(2), ny/T(2)});
  SmoothIndicatorCuboid2D<T,T> section2( ind2, 0. );

  AnalyticalIdentity2D<T,T> c1( section1 );
  AnalyticalIdentity2D<T,T> c2( section2 );
  AnalyticalIdentity2D<T,T> rho( one );
  AnalyticalIdentity2D<T,T> phi( c1 - c2 );
  AnalyticalIdentity2D<T,T> psi( rho - c1 - c2 );

  auto allIndicator = superGeometry.getMaterialIndicator({1, 2, 3, 4, 5, 6});
  sLattice1.iniEquilibrium( allIndicator, rho, zeroVelocity );
  sLattice2.iniEquilibrium( allIndicator, phi, zeroVelocity );
  sLattice3.iniEquilibrium( allIndicator, psi, zeroVelocity );

  // inlet boundary conditions
  clout << "Prepare Lattice: Inlet boundary conditions ..." << std::endl;
  Poiseuille2D<T> inlet1U( superGeometry, 3, 1.5*inlet1Velocity, 0. );
  sLattice1.defineU( inlet1Indicator, inlet1U );
  sLattice2.defineRho( inlet1Indicator, phi );
  sLattice3.defineRho( inlet1Indicator, psi );

  Poiseuille2D<T> inlet21U( superGeometry, 4, 1.5*inlet2Velocity, 0. );
  Poiseuille2D<T> inlet22U( superGeometry, 5, 1.5*inlet2Velocity, 0. );
  sLattice1.defineU( superGeometry, 4, inlet21U );
  sLattice1.defineU( superGeometry, 5, inlet22U );
  sLattice2.defineRho( inlet2Indicator, phi );
  sLattice3.defineRho( inlet2Indicator, psi );

  Poiseuille2D<T> inlet31U( superGeometry, 6, 1.5*inlet3Velocity, 0. );
  Poiseuille2D<T> inlet32U( superGeometry, 7, 1.5*inlet3Velocity, 0. );
  sLattice1.defineU( superGeometry, 6, inlet31U );
  sLattice1.defineU( superGeometry, 7, inlet32U );
  sLattice2.defineRho( inlet3Indicator, phi );
  sLattice3.defineRho( inlet3Indicator, psi );

  // outlet initial / boundary conditions
  clout << "Prepare Lattice: Outlet initial / Boundary conditions ..." << std::endl;
  AnalyticalConst2D<T,T> rhoOutlet( outletDensity );
  AnalyticalIdentity2D<T,T> phiOutlet( zero );
  AnalyticalIdentity2D<T,T> psiOutlet( rhoOutlet );
  sLattice1.defineRho( outletIndicator, rhoOutlet );
  sLattice2.defineRho( outletIndicator, phiOutlet );
  sLattice3.defineRho( outletIndicator, psiOutlet );

  sLattice1.setParameter<descriptors::OMEGA>(omega);
  sLattice2.setParameter<descriptors::OMEGA>(omega);
  sLattice2.setParameter<collision::FreeEnergy::GAMMA>(gama);
  sLattice3.setParameter<descriptors::OMEGA>(omega);
  sLattice3.setParameter<collision::FreeEnergy::GAMMA>(gama);

  // initialise lattices
  clout << "Prepare Lattice: Initialise lattices ..." << std::endl;
  sLattice1.initialize();
  sLattice2.initialize();
  sLattice3.initialize();

  clout << "Prepare Lattice: Communicate ..." << std::endl;
  sLattice1.communicate();
  sLattice2.communicate();
  sLattice3.communicate();

  {
    auto& communicator = sLattice1.getCommunicator(stage::PostPostProcess());
    communicator.requestField<POPULATION>();
    communicator.requestOverlap(sLattice1.getOverlap());
    communicator.exchangeRequests();
  }
  {
    auto& communicator = sLattice2.getCommunicator(stage::PostPostProcess());
    communicator.requestField<POPULATION>();
    communicator.requestOverlap(sLattice2.getOverlap());
    communicator.exchangeRequests();
  }
  {
    auto& communicator = sLattice3.getCommunicator(stage::PostPostProcess());
    communicator.requestField<POPULATION>();
    communicator.requestOverlap(sLattice3.getOverlap());
    communicator.exchangeRequests();
  }

  clout << "Prepare Lattice ... OK" << std::endl;
}

void getResults( SuperLattice<T, DESCRIPTOR>& sLattice1,
                 SuperLattice<T, DESCRIPTOR>& sLattice2,
                 SuperLattice<T, DESCRIPTOR>& sLattice3, int iT,
                 SuperGeometry<T,2>& superGeometry, util::Timer<T>& timer,
                 UnitConverter<T, DESCRIPTOR> converter)
{

  OstreamManager clout( std::cout,"getResults" );
  SuperVTMwriter2D<T> vtmWriter( "microFluidics2d" );

  if ( iT==0 ) {
    // Writes the geometry, cuboid no. and rank no. as vti file for visualization
    SuperLatticeCuboid2D<T, DESCRIPTOR> cuboid( sLattice1 );
    SuperLatticeRank2D<T, DESCRIPTOR> rank( sLattice1 );
    vtmWriter.write( cuboid );
    vtmWriter.write( rank );
    vtmWriter.createMasterFile();
  }

  // Get statistics
  if ( iT%statIter==0 ) {
    // Timer console output
    timer.update( iT );
    timer.printStep();
    sLattice1.getStatistics().print( iT, converter.getPhysTime(iT) );
    sLattice2.getStatistics().print( iT, converter.getPhysTime(iT) );
    sLattice3.getStatistics().print( iT, converter.getPhysTime(iT) );
  }

  // Writes the VTK files
  if ( iT%vtkIter==0 ) {
    sLattice1.setProcessingContext(ProcessingContext::Evaluation);
    SuperLatticeVelocity2D<T, DESCRIPTOR> velocity( sLattice1 );
    SuperLatticeDensity2D<T, DESCRIPTOR> density1( sLattice1 );
    density1.getName() = "rho";
    SuperLatticeDensity2D<T, DESCRIPTOR> density2( sLattice2 );
    density2.getName() = "phi";
    SuperLatticeDensity2D<T, DESCRIPTOR> density3( sLattice3 );
    density3.getName() = "density-fluid-3";

    AnalyticalConst2D<T,T> half_( 0.5 );
    SuperLatticeFfromAnalyticalF2D<T, DESCRIPTOR> half(half_, sLattice1);

    SuperIdentity2D<T,T> c1 (half*(density1+density2-density3));
    c1.getName() = "density-fluid-1";
    SuperIdentity2D<T,T> c2 (half*(density1-density2-density3));
    c2.getName() = "density-fluid-2";

    vtmWriter.addFunctor( velocity );
    vtmWriter.addFunctor( density1 );
    vtmWriter.addFunctor( density2 );
    vtmWriter.addFunctor( density3 );
    vtmWriter.addFunctor( c1 );
    vtmWriter.addFunctor( c2 );
    vtmWriter.write( iT );
  }
}


int main( int argc, char *argv[] )
{

  // === 1st Step: Initialization ===

  initialize( &argc, &argv );
  singleton::directories().setOutputDir( "./tmp/" );
  OstreamManager clout( std::cout,"main" );

  UnitConverterFromResolutionAndRelaxationTime<T,DESCRIPTOR> converter(
    (T)   N, // resolution
    (T)   1., // lattice relaxation time (tau)
    (T)   ny, // charPhysLength: reference length of simulation geometry
    (T)   1.e-6, // charPhysVelocity: maximal/highest expected velocity during simulation in __m / s__
    (T)   0.1, // physViscosity: physical kinematic viscosity in __m^2 / s__
    (T)   1. // physDensity: physical density in __kg / m^3__
  );

  // Prints the converter log as console output
  converter.print();

  // === 2nd Step: Prepare Geometry ===
  std::vector<T> extend = { nx, ny };
  std::vector<T> origin = { 0, 0 };
  IndicatorCuboid2D<T> cuboid(extend,origin);
#ifdef PARALLEL_MODE_MPI
  CuboidDecomposition2D<T> cuboidDecomposition( cuboid, converter.getPhysDeltaX(), singleton::mpi().getSize() );
#else
  CuboidDecomposition2D<T> cuboidDecomposition( cuboid, converter.getPhysDeltaX() );
#endif

  // Instantiation of loadbalancer
  HeuristicLoadBalancer<T> loadBalancer( cuboidDecomposition );
  loadBalancer.print();

  // Instantiation of superGeometry
  SuperGeometry<T,2> superGeometry( cuboidDecomposition,loadBalancer );

  prepareGeometry( superGeometry );

  // === 3rd Step: Prepare Lattice ===
  SuperLattice<T, DESCRIPTOR> sLattice1( superGeometry );
  SuperLattice<T, DESCRIPTOR> sLattice2( superGeometry );
  SuperLattice<T, DESCRIPTOR> sLattice3( superGeometry );

  //prepareLattice and set boundaryConditions
  prepareLattice( sLattice1, sLattice2, sLattice3, converter, superGeometry );

  // Prepare Coupling
  clout << "Add lattice coupling" << std::endl;

  SuperLatticeCoupling coupling1(
  DensityOutletCoupling2D{},
  names::A{}, sLattice1,
  names::B{}, sLattice2,
  names::C{}, sLattice3
  );

  coupling1.template setParameter<DensityOutletCoupling2D::RHO>(outletDensity);

  coupling1.restrictTo(superGeometry.getMaterialIndicator({8}));


  SuperLatticeCoupling coupling2(
  ChemicalPotentialCoupling2D{},
  names::A{}, sLattice1,
  names::B{}, sLattice2,
  names::C{}, sLattice3
  );

  coupling2.template setParameter<ChemicalPotentialCoupling2D::ALPHA>(alpha);
  coupling2.template setParameter<ChemicalPotentialCoupling2D::KAPPA1>(kappa1);
  coupling2.template setParameter<ChemicalPotentialCoupling2D::KAPPA2>(kappa2);
  coupling2.template setParameter<ChemicalPotentialCoupling2D::KAPPA2>(kappa3);
  coupling2.restrictTo(superGeometry.getMaterialIndicator({1}));

  SuperLatticeCoupling coupling3(
  ForceCoupling2D{},
  names::A{}, sLattice2,
  names::B{}, sLattice1,
  names::C{}, sLattice3
  );
  coupling3.restrictTo(superGeometry.getMaterialIndicator({1}));


  SuperLatticeCoupling coupling4(
  InletOutletCoupling2D{},
  names::A{}, sLattice2,
  names::B{}, sLattice1,
  names::C{}, sLattice3
  );
  coupling4.restrictTo(superGeometry.getMaterialIndicator({3,4,5,6,7,8}));

  sLattice1.addPostProcessor<stage::PreCoupling>(meta::id<RhoStatistics>());
  sLattice2.addPostProcessor<stage::PreCoupling>(meta::id<RhoStatistics>());
  sLattice3.addPostProcessor<stage::PreCoupling>(meta::id<RhoStatistics>());



  {
    auto& communicator = sLattice1.getCommunicator(stage::PostCoupling());
    communicator.requestField<CHEM_POTENTIAL>();
    communicator.requestField<RhoStatistics>();
    communicator.requestOverlap(sLattice1.getOverlap());
    communicator.exchangeRequests();
  }
  {
    auto& communicator = sLattice2.getCommunicator(stage::PreCoupling());
    communicator.requestField<CHEM_POTENTIAL>();
    communicator.requestField<RhoStatistics>();
    communicator.requestOverlap(sLattice2.getOverlap());
    communicator.exchangeRequests();
  }
  {
    auto& communicator = sLattice3.getCommunicator(stage::PreCoupling());
    communicator.requestField<CHEM_POTENTIAL>();
    communicator.requestField<RhoStatistics>();
    communicator.requestOverlap(sLattice3.getOverlap());
    communicator.exchangeRequests();
  }

  clout << "Add lattice coupling ... OK!" << std::endl;


  // === 4th Step: Main Loop with Timer ===
  int iT = 0;
  clout << "starting simulation..." << std::endl;
  util::Timer<T> timer( maxIter, superGeometry.getStatistics().getNvoxel() );
  timer.start();

  for ( iT=0; iT<maxIter; ++iT ) {
    // Computation and output of the results
    getResults( sLattice1, sLattice2, sLattice3, iT, superGeometry, timer, converter );

    // Collide and stream execution
    sLattice1.collideAndStream();
    sLattice2.collideAndStream();
    sLattice3.collideAndStream();

    sLattice1.executePostProcessors(stage::PreCoupling());
    sLattice2.executePostProcessors(stage::PreCoupling());
    sLattice3.executePostProcessors(stage::PreCoupling());

    sLattice1.getCommunicator(stage::PreCoupling()).communicate();
    sLattice2.getCommunicator(stage::PreCoupling()).communicate();
    sLattice3.getCommunicator(stage::PreCoupling()).communicate();

    // Execute coupling between the two lattices
    coupling1.execute();
    coupling2.execute();

    sLattice1.getCommunicator(stage::PostCoupling()).communicate();
    sLattice1.executePostProcessors(stage::PostCoupling());


    coupling3.execute();
    coupling4.execute();




  }

  timer.stop();
  timer.printSummary();

}
