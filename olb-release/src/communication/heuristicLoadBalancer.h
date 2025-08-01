/*  This file is part of the OpenLB library
 *
 *  Copyright (C) 2012 Jonas Fietz, Mathias J. Krause
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


#ifndef HEURISTIC_LOAD_BALANCER_H
#define HEURISTIC_LOAD_BALANCER_H


#include "communication/mpiManager.h"
#include "geometry/cuboid.h"
#include "geometry/cuboidDecomposition.h"
#include "geometry/cuboidDecomposition.hh"
#include "communication/loadBalancer.h"




namespace olb {

/** Constructs a load balancer from a given cuboid geometry using a heurist.
 * \param cGeometry     cuboid geometry to base the load balance on
 * \param blockGeometry used to determine number of full and empty cells if given
 * \param ratioFullEmpty time it takes for full cells in relation to empty cells
 *
 * This class has a virtual method call in its destructor and should therefore
 * not be used as a base class.
 */
template<typename T>
class HeuristicLoadBalancer final : public LoadBalancer<T> {
private:
  // Handles the MPI communication
#ifdef PARALLEL_MODE_MPI
  singleton::MpiNonBlockingHelper _mpiNbHelper;
#endif
  CuboidDecomposition3D<T>* _cGeometry3d;
  CuboidDecomposition2D<T>* _cGeometry2d;

  double _ratioFullEmpty;

public:
  HeuristicLoadBalancer() {};
  ~HeuristicLoadBalancer() override;

  HeuristicLoadBalancer(CuboidDecomposition3D<T>& cGeometry3d, const double ratioFullEmpty=1., const double weightEmpty=.0);
  HeuristicLoadBalancer(CuboidDecomposition2D<T>& cGeometry2d, const double ratioFullEmpty=1., const double weightEmpty=.0);

  void reInit(CuboidDecomposition3D<T>& cGeometry3d, const double ratioFullEmpty=1., const double weightEmpty=.0);
  void reInit(CuboidDecomposition2D<T>& cGeometry2d, const double ratioFullEmpty=1., const double weightEmpty=.0);

  void swap(HeuristicLoadBalancer<T>& loadBalancer);

};
}  // namespace olb

#endif
