/*  This file is part of the OpenLB library
 *
 *  Copyright (C) 2024 Dennis Teutscher
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

#ifndef OLB_BLOCK_REDUCTION_3D1D_H
#define OLB_BLOCK_REDUCTION_3D1D_H

#include "core/blockData.h"
#include "core/vector.h"
#include "blockBaseF2D.h"
#include "superBaseF2D.h"
#include "utilities/line3D.h"
#include "utilities/lineLattice3D.h"
#include "utilities/functorPtr.h"
#include "utilities/blockDataSyncMode.h"
#include "utilities/blockDataReductionMode.h"

#include <tuple>


namespace olb {


/// BlockReduction3D1D reduces the data of a SuperF3D functor to the
/// intersection between a given 3D line3D and the super geometry.
/**
 * This intersection is interpolated at a set of discrete points according to
 * the given resolution and exposed as a 1-dimensional BlockDataF2D functor.
 *
 * The line3D is parametrized by a origin and a single direction vector u.
 * Definition of line3D using e.g. origin and normal vectors is supported
 * via the line3D interface.
 **/
template <typename T>
class BlockReduction3D1D final : public LineLattice3D<T>, public BlockDataF2D<T,T> {
private:
  /// Functor to be reduced
  FunctorPtr<SuperF3D<T>> _f;
  /// Plane points scheduled for storage in _blockData
  /// i.e. Plane points whose physical location intersects the mother cuboid
  ///      and is nearest to a rank-local cuboid
  std::vector<std::tuple<int,int>> _rankLocalSubplane;
  /// Synchronization mode, see BlockDataSyncMode enum for further information.
  /// This value only matters when PARALLEL_MODE_MPI is defined.
  const BlockDataSyncMode _syncMode;
  /// Reduction mode, see BlockDataReductionMode enum for further information.
  const BlockDataReductionMode _reductionMode;

  void updateBlockAnalytical(BlockData<2,T,T>& block);
  void updateBlockDiscrete(BlockData<2,T,T>& block);

public:
  /// Construction using functor and line3D lattice
  /**
   * \param f Functor to be reduced as a (non-)owning pointer or reference to SuperF3D<T>.
   * \param lattice Line3D lattice parametrization
   * \param syncMode
   *        Defines MPI synchronization strategy of the interpolated block data.
   * \param reductionMode
   *        Defines whether data is interpolated or read from discrete lattice locations.
   *        Note: BlockDataReductionMode::Analytical imposes restrictions on line3D
   *        definition and discretization.
   **/
  BlockReduction3D1D(FunctorPtr<SuperF3D<T>>&& f,
                     const LineLattice3D<T>& lattice,
                     BlockDataSyncMode syncMode=BlockDataSyncMode::ReduceAndBcast,
                     BlockDataReductionMode reductionMode=BlockDataReductionMode::Analytical);
  /// Construction using functor and line3D
  /**
   * \param f Functor to be reduced as a (non-)owning pointer or reference to SuperF3D<T>.
   * \param line3D Line3D parametrization
   * \param syncMode
   *        Defines MPI synchronization strategy of the interpolated block data.
   * \param reductionMode
   *        Defines whether data is interpolated or read from discrete lattice locations.
   **/
  BlockReduction3D1D(FunctorPtr<SuperF3D<T>>&& f,
                     const Line3D<T>& line3D,
                     BlockDataSyncMode syncMode=BlockDataSyncMode::ReduceAndBcast,
                     BlockDataReductionMode reductionMode=BlockDataReductionMode::Analytical);
  /// Construction using functor, line3D and resolution
  /**
   * \param f Functor to be reduced as a (non-)owning pointer or reference to SuperF3D<T>.
   * \param line3D Line3D parametrization
   * \param resolution Defines the number of voxel of the longest side.
   *                   If it equals zero, _h is set to the cuboid geometry's minDeltaR.
   * \param mode       Defines MPI synchronization strategy of the interpolated block data.
   **/
  BlockReduction3D1D(FunctorPtr<SuperF3D<T>>&& f,
                     const Line3D<T>& line3D,
                     int resolution=600,
                     BlockDataSyncMode mode=BlockDataSyncMode::ReduceAndBcast);

  /// Construction using functor, origin, direction and resolution
  /**
   * \param f Functor to be reduced as a (non-)owning pointer or reference to SuperF3D<T>.
   * \param direction Direction vector
   * \param origin    Origin vector
   * \param resolution Defines the number of voxel of the longest side.
   *                   If it equals zero, _h is set to the cuboid geometry's minDeltaR.
   * \param mode Defines MPI synchronization strategy of the interpolated block data.
   **/
  BlockReduction3D1D(FunctorPtr<SuperF3D<T>>&& f,
                     const Vector<T,3>& origin, const Vector<T,3>& direction,
                     int resolution=600,
                     BlockDataSyncMode mode=BlockDataSyncMode::ReduceAndBcast);

  /// Custom operator for easier access to 1-dimensional block data
  bool operator() (T output[], int i);
  /// Initialize rank-local list of plane points to be stored in _blockData
  void initialize();
  /// Updates and writes the data to _blockData using _rankLocalSubplane
  void update();
  /// Overload of virtual function from class BlockF2D
  BlockStructureD<2>& getBlockStructure() override;
  /// \return reference to the rank local list of discrete line points, cuboid ids
  const std::vector<std::tuple<int,int>>& getRankLocalSubplane() const;

};


} // end namespace olb

#endif
