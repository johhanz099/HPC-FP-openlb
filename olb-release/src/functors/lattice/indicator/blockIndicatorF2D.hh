/*  This file is part of the OpenLB library
 *
 *  Copyright (C) 2017 Adrian Kummerlaender
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

#ifndef BLOCK_INDICATOR_F_2D_HH
#define BLOCK_INDICATOR_F_2D_HH

#include <algorithm>

#include "blockIndicatorF2D.h"
#include "core/util.h"

namespace olb {

template <typename T>
BlockIndicatorFfromIndicatorF2D<T>::BlockIndicatorFfromIndicatorF2D(
  IndicatorF2D<T>& indicatorF, BlockGeometry<T,2>& blockGeometry)
  : BlockIndicatorF2D<T>(blockGeometry),
    _indicatorF(indicatorF)
{ }

template <typename T>
bool BlockIndicatorFfromIndicatorF2D<T>::operator() (bool output[], const int input[])
{
  auto physR = this->_blockGeometryStructure.getPhysR(input);
  return _indicatorF(output, physR.data());
}

template <typename T>
Vector<int,2> BlockIndicatorFfromIndicatorF2D<T>::getMin()
{
  const Vector<T,2> min = _indicatorF.getMin();
  return Vector<int,2> {
    static_cast<int>(util::floor(min[0])),
    static_cast<int>(util::floor(min[1]))
  };
}

template <typename T>
Vector<int,2> BlockIndicatorFfromIndicatorF2D<T>::getMax()
{
  const Vector<T,2> max = _indicatorF.getMax();
  return Vector<int,2> {
    static_cast<int>(util::ceil(max[0])),
    static_cast<int>(util::ceil(max[1]))
  };
}


template <typename T, bool HLBM>
BlockIndicatorFfromSmoothIndicatorF2D<T,HLBM>::BlockIndicatorFfromSmoothIndicatorF2D(
  SmoothIndicatorF2D<T,T,HLBM>& indicatorF, BlockGeometry<T,2>& blockGeometry)
  : BlockIndicatorF2D<T>(blockGeometry),
    _indicatorF(indicatorF)
{ }

template <typename T, bool HLBM>
bool BlockIndicatorFfromSmoothIndicatorF2D<T,HLBM>::operator() (bool output[], const int input[])
{
  T inside[1];
  auto physR = this->_blockGeometryStructure.getPhysR(input);
  _indicatorF(inside, physR.data());
  return !util::nearZero(inside[0]);
}

template <typename T, bool HLBM>
Vector<int,2> BlockIndicatorFfromSmoothIndicatorF2D<T,HLBM>::getMin()
{
  return Vector<int,2> {0,0};
}

template <typename T, bool HLBM>
Vector<int,2> BlockIndicatorFfromSmoothIndicatorF2D<T,HLBM>::getMax()
{
  return this->_blockGeometryStructure.getExtent() - Vector<int,2> {1,1};
}


template <typename T>
BlockIndicatorMaterial2D<T>::BlockIndicatorMaterial2D(
  BlockGeometry<T,2>& blockGeometry, std::vector<int> materials)
  : BlockIndicatorF2D<T>(blockGeometry),
    _materials(materials)
{ }

template <typename T>
BlockIndicatorMaterial2D<T>::BlockIndicatorMaterial2D(
  BlockGeometry<T,2>& blockGeometry, std::list<int> materials)
  : BlockIndicatorMaterial2D(blockGeometry,
                             std::vector<int>(materials.begin(), materials.end()))
{ }

template <typename T>
BlockIndicatorMaterial2D<T>::BlockIndicatorMaterial2D(
  BlockGeometry<T,2>& blockGeometry, int material)
  : BlockIndicatorMaterial2D(blockGeometry, std::vector<int>(1,material))
{ }

template <typename T>
bool BlockIndicatorMaterial2D<T>::operator() (bool output[], const int input[])
{
  // read material number explicitly using the const version
  // of BlockGeometry<T,2>::get to avoid resetting geometry
  // statistics:
  const BlockGeometry<T,2>& blockGeometry = this->_blockGeometryStructure;
  const int current = blockGeometry.getMaterial({input[0], input[1]});
  output[0] = std::any_of(_materials.cbegin(),
                          _materials.cend(),
  [current](int material) {
    return current == material;
  });

  return true;
}

template <typename T>
bool BlockIndicatorMaterial2D<T>::isEmpty()
{
  const auto& statistics = this->getBlockGeometry().getStatistics();
  return std::none_of(_materials.cbegin(), _materials.cend(),
                      [&statistics](int material) -> bool {
                        return statistics.getNvoxel(material) > 0;
                      });
}

template <typename T>
Vector<int,2> BlockIndicatorMaterial2D<T>::getMin()
{
  const auto& blockGeometry = this->getBlockGeometry();
  const auto& statistics    = blockGeometry.getStatistics();

  Vector<int,2> globalMin{
    blockGeometry.getNx()+blockGeometry.getPadding()-1,
    blockGeometry.getNy()+blockGeometry.getPadding()-1
  };

  for ( int material : _materials ) {
    if ( statistics.getNvoxel(material) > 0 ) {
      const Vector<int,2> localMin = statistics.getMinLatticeR(material);
      for ( int d = 0; d < 2; ++d ) {
        globalMin[d] = localMin[d] < globalMin[d] ? localMin[d] : globalMin[d];
      }
    }
  }

  return globalMin;
}

template <typename T>
Vector<int,2> BlockIndicatorMaterial2D<T>::getMax()
{
  const auto& statistics = this->getBlockGeometry().getStatistics();

  Vector<int,2> globalMax = -this->getBlockGeometry().getPadding();

  for ( int material : _materials ) {
    if ( statistics.getNvoxel(material) > 0 ) {

      const Vector<int,2> localMax = statistics.getMaxLatticeR(material);
      for ( int d = 0; d < 2; ++d ) {
        globalMax[d] = localMax[d] > globalMax[d] ? localMax[d] : globalMax[d];
      }
    }
  }

  return globalMax;
}


template <typename T>
BlockIndicatorIdentity2D<T>::BlockIndicatorIdentity2D(BlockIndicatorF2D<T>& indicatorF)
  : BlockIndicatorF2D<T>(indicatorF.getBlockGeometry()),
    _indicatorF(indicatorF)
{ }

template <typename T>
bool BlockIndicatorIdentity2D<T>::operator() (bool output[], const int input[])
{
  return _indicatorF(output, input);
}

template <typename T>
Vector<int,2> BlockIndicatorIdentity2D<T>::getMin()
{
  return _indicatorF.getMin();
}

template <typename T>
Vector<int,2> BlockIndicatorIdentity2D<T>::getMax()
{
  return _indicatorF.getMax();
}


template <typename T>
BlockIndicatorBoundaryNeighbor2D<T>::BlockIndicatorBoundaryNeighbor2D(BlockIndicatorF2D<T>& indicatorF, int overlap)
  : BlockIndicatorF2D<T>(indicatorF.getBlockGeometry()),
    _indicatorF(indicatorF),
    _overlap(overlap)
{ }

template <typename T>
bool BlockIndicatorBoundaryNeighbor2D<T>::operator() (bool output[], const int input[])
{
  // check if current position is not solid
  if ( this->getBlockGeometry().getMaterial(input[0],input[1]) != 0 ) {
    // check all neighbors if they are part of boundary via indicator
    for ( int iXo = -_overlap; iXo <= _overlap; ++iXo ) {
      for ( int iYo = -_overlap; iYo <= _overlap; ++iYo ) {
        const int neighborPos[2] = {iXo + input[0], iYo + input[1]};
        if ( _indicatorF ( neighborPos ) &&
             this->getBlockGeometry().isInside( neighborPos ) ) {
          output[0] = true;
          return true;
        }
      }
    }
  }
  return true;
}

template <typename T>
Vector<int,2> BlockIndicatorBoundaryNeighbor2D<T>::getMin()
{
  return _indicatorF.getMin() - _overlap;
}

template <typename T>
Vector<int,2> BlockIndicatorBoundaryNeighbor2D<T>::getMax()
{
  return _indicatorF.getMax() + _overlap;
}

} // namespace olb

#endif
