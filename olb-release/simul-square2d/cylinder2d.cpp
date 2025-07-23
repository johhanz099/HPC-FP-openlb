/*  Lattice Boltzmann sample, written in C++, using the OpenLB library
 *
 *  Copyright (C) 2006-2019 Jonas Latt, Mathias J. Krause,
 *  Vojtech Cvrcek, Peter Weisbrod, Adrian Kummerländer
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
 *  You should have received a copy of the GNU General
 *  Public License along with this program; if not, write to the Free
 *  Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 */

#include <olb.h>

using namespace olb;
using namespace olb::descriptors;
using namespace olb::graphics;

using T = FLOATING_POINT_TYPE;
using DESCRIPTOR = D2Q9<>;

#define BOUZIDI

// Parameters for the simulation setup
const int N = 10;       // resolution of the model
const T CFL = 0.05;     // characteristic CFL number
const T Re = 20.;       // Reynolds number
const T maxPhysT = 16;  // max. simulation time in s, SI unit
const T L = 0.1/N;      // latticeL
const T lengthX = 2.2;
const T lengthY = .41+L;
const T centerCylinderX = 0.2;
const T centerCylinderY = 0.2+L/2.;
const T radiusCylinder = 0.05;

// Stores geometry information in form of material numbers
void prepareGeometry( UnitConverter<T, DESCRIPTOR> const& converter,
                      SuperGeometry<T,2>& superGeometry,
                      std::shared_ptr<IndicatorF2D<T>> circle)
{
  OstreamManager clout( std::cout,"prepareGeometry" );
  clout << "Prepare Geometry ..." << std::endl;

  Vector<T,2> extend( lengthX,lengthY );
  Vector<T,2> origin;

  superGeometry.rename( 0,2 );
  superGeometry.rename( 2,1,{1,1} );

  // Set material number for inflow
  extend[0] = 2.*L;
  origin[0] = -L;
  IndicatorCuboid2D<T> inflow( extend, origin );
  superGeometry.rename( 2,3,1,inflow );
  // Set material number for outflow
  origin[0] = lengthX-L;
  IndicatorCuboid2D<T> outflow( extend, origin );
  superGeometry.rename( 2,4,1,outflow );
  // Set material number for obstacle   // CHANGED: was cylinder
  superGeometry.rename( 1,5, circle );

  superGeometry.clean();
  superGeometry.checkForErrors();

  superGeometry.print();
  clout << "Prepare Geometry ... OK" << std::endl;
}

// Set up the geometry of the simulation
void prepareLattice( SuperLattice<T,DESCRIPTOR>& sLattice,
                     UnitConverter<T, DESCRIPTOR> const& converter,
                     SuperGeometry<T,2>& superGeometry,
                     std::shared_ptr<IndicatorF2D<T>> circle)
{
  OstreamManager clout( std::cout,"prepareLattice" );
  clout << "Prepare Lattice ..." << std::endl;

  const T omega = converter.getLatticeRelaxationFrequency();

  auto bulkIndicator = superGeometry.getMaterialIndicator({1});
  sLattice.defineDynamics<BGKdynamics>(bulkIndicator);

  boundary::set<boundary::BounceBack>(sLattice, superGeometry, 2);
  boundary::set<boundary::InterpolatedVelocity>(sLattice, superGeometry, 3);
  boundary::set<boundary::InterpolatedPressure>(sLattice, superGeometry, 4);

  #ifdef BOUZIDI
  setBouzidiBoundary(sLattice, superGeometry, 5, *circle);
  #else
  boundary::set<boundary::BounceBack>(sLattice, superGeometry, 5);
  #endif

  AnalyticalConst2D<T,T> rhoF( 1 );
  std::vector<T> velocity( 2,T( 0 ) );
  AnalyticalConst2D<T,T> uF( velocity );

  sLattice.defineRhoU( bulkIndicator, rhoF, uF );
  sLattice.iniEquilibrium( bulkIndicator, rhoF, uF );
  sLattice.setParameter<descriptors::OMEGA>(omega);
  sLattice.initialize();

  clout << "Prepare Lattice ... OK" << std::endl;
}

// Generates a slowly increasing inflow for the first iTMaxStart timesteps
void setBoundaryValues( SuperLattice<T, DESCRIPTOR>& sLattice,
                        UnitConverter<T, DESCRIPTOR> const& converter, int iT,
                        SuperGeometry<T,2>& superGeometry)
{
  OstreamManager clout( std::cout,"setBoundaryValues" );

  int iTmaxStart = converter.getLatticeTime( maxPhysT*0.4 );
  int iTupdate = 5;

  if ( iT%iTupdate==0 && iT<= iTmaxStart ) {
    PolynomialStartScale<T,T> StartScale( iTmaxStart, T( 1 ) );
    T iTvec[1] = {T( iT )}; T frac[1] = {};
    StartScale( frac,iTvec );
    T maxVelocity = converter.getCharLatticeVelocity()*3./2.*frac[0];
    T distance2Wall = L/2.;
    Poiseuille2D<T> poiseuilleU( superGeometry, 3, maxVelocity, distance2Wall );

    sLattice.defineU( superGeometry, 3, poiseuilleU );
    sLattice.setProcessingContext<Array<momenta::FixedVelocityMomentumGeneric::VELOCITY>>(ProcessingContext::Simulation);
  }
}

// Computes the pressure drop between the voxels before and after the obstacle
void getResults( SuperLattice<T, DESCRIPTOR>& sLattice,
                 UnitConverter<T, DESCRIPTOR> const& converter, std::size_t iT,
                 SuperGeometry<T,2>& superGeometry, util::Timer<T>& timer )
{
  OstreamManager clout( std::cout,"getResults" );
  static Gnuplot<T> gplot( "drag" );

  SuperVTMwriter2D<T> vtmWriter( "cylinder2d" );
  SuperLatticePhysVelocity2D<T, DESCRIPTOR> velocity( sLattice, converter );
  SuperLatticePhysPressure2D<T, DESCRIPTOR> pressure( sLattice, converter );

  vtmWriter.addFunctor( velocity );
  vtmWriter.addFunctor( pressure );

  const int vtkIter  = converter.getLatticeTime( .3 );
  const int statIter = converter.getLatticeTime( .1 );

  T point[2] = { centerCylinderX + 3*radiusCylinder, centerCylinderY };
  AnalyticalFfromSuperF2D<T> intpolateP( pressure, true );
  T p; intpolateP( &p,point );

  if ( iT == 0 ) {
    SuperLatticeCuboid2D<T, DESCRIPTOR> cuboid( sLattice );
    SuperLatticeRank2D<T, DESCRIPTOR> rank( sLattice );
    vtmWriter.write( cuboid ); vtmWriter.write( rank ); vtmWriter.createMasterFile();
  }

  if ( iT%statIter == 0 ) {
    sLattice.setProcessingContext(ProcessingContext::Evaluation);
    timer.update( iT ); timer.printStep();
    sLattice.getStatistics().print( iT,converter.getPhysTime( iT ) );

    AnalyticalFfromSuperF2D<T> intpolatePressure( pressure, true );
    SuperLatticePhysDrag2D<T,DESCRIPTOR> drag( sLattice, superGeometry, 5, converter );
    T point1[2] = { centerCylinderX - radiusCylinder, centerCylinderY };
    T point2[2] = { centerCylinderX + radiusCylinder, centerCylinderY };
    T p1,p2; intpolatePressure( &p1,point1 ); intpolatePressure( &p2,point2 );
    clout << "; pressureDrop=" << p1-p2;
    int input[3] = {}; T _drag[drag.getTargetDim()]; drag( _drag,input );
    clout << "; drag=" << _drag[0] << "; lift=" << _drag[1] << std::endl;
    gplot.setData( converter.getPhysTime( iT ), {_drag[0], 5.58}, {"drag(openLB)", "drag(schaeferTurek)"}, "bottom right", {'l','l'} );
    if ( iT%( vtkIter ) == 0 ) gplot.writePNG( iT, maxPhysT );
  }

  if ( iT%vtkIter == 0 && iT > 0 ) {
    vtmWriter.write( iT );
    SuperEuklidNorm2D<T, DESCRIPTOR> normVel( velocity );
    BlockReduction2D2D<T> planeReduction( normVel, 600, BlockDataSyncMode::ReduceOnly );
    heatmap::write(planeReduction, iT);
  }

  if ( iT == converter.getLatticeTime( maxPhysT )-1 ) {
    gplot.writePDF();
  }
}

int main( int argc, char* argv[] )
{
  initialize( &argc, &argv );
  singleton::directories().setOutputDir( "./tmp/" );
  OstreamManager clout( std::cout,"main" );

  UnitConverterFromResolutionAndRelaxationTime<T, DESCRIPTOR> const converter(
    int {N}, (T)0.56, (T)2.0*radiusCylinder, (T)0.2,
    (T)0.2*2.*radiusCylinder/Re, (T)1.0
  );
  converter.print(); converter.write("cylinder2d");

  Vector<T,2> extend( lengthX, lengthY );
  Vector<T,2> origin;
  IndicatorCuboid2D<T> cuboid( extend, origin );

  #ifdef PARALLEL_MODE_MPI
  const int noOfCuboids = singleton::mpi().getSize();
  #else
  const int noOfCuboids = 7;
  #endif
  CuboidDecomposition2D<T> cuboidDecomposition( cuboid, L, noOfCuboids );
  HeuristicLoadBalancer<T> loadBalancer( cuboidDecomposition );
  SuperGeometry<T,2> superGeometry( cuboidDecomposition, loadBalancer );

  // CHANGED: create rectangle instead of circle
  Vector<T,2> size( 2*radiusCylinder, 2*radiusCylinder );
  Vector<T,2> rectOrigin( centerCylinderX-radiusCylinder, centerCylinderY-radiusCylinder );
  auto rectangle = std::make_shared<IndicatorCuboid2D<T>>( size, rectOrigin );

  prepareGeometry( converter, superGeometry, rectangle );
  SuperLattice<T, DESCRIPTOR> sLattice( superGeometry );
  prepareLattice( sLattice, converter, superGeometry, rectangle );

  clout << "starting simulation..." << std::endl;
  util::Timer<T> timer( converter.getLatticeTime( maxPhysT ), superGeometry.getStatistics().getNvoxel() );
  timer.start();
  for ( std::size_t iT = 0; iT < converter.getLatticeTime( maxPhysT ); ++iT ) {
    setBoundaryValues( sLattice, converter, iT, superGeometry );
    sLattice.collideAndStream();
    getResults( sLattice, converter, iT, superGeometry, timer );
  }
  timer.stop(); timer.printSummary();
}
