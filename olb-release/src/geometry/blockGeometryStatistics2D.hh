/*  This file is part of the OpenLB library
 *
 *  Copyright (C) 2011, 2014 Mathias J. Krause, Simon Zimny
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

/** \file
 * Representation of a statistic for a 2D geometry -- generic implementation.
 */

#ifndef BLOCK_GEOMETRY_STATISTICS_2D_HH
#define BLOCK_GEOMETRY_STATISTICS_2D_HH

#include <iostream>
#include <math.h>
#include <fstream>
#include <sstream>
#include "utilities/omath.h"

#include "geometry/blockGeometry.h"
#include "geometry/blockGeometryStatistics2D.h"

namespace olb {

template<typename T>
BlockGeometryStatistics2D<T>::BlockGeometryStatistics2D(BlockGeometry<T,2>* blockGeometry)
  : _blockGeometry(blockGeometry),
    clout(std::cout,"BlockGeometryStatistics2D")
{
  _statisticsUpdateNeeded = true;
}


template<typename T>
bool& BlockGeometryStatistics2D<T>::getStatisticsStatus()
{
  return _statisticsUpdateNeeded;
}

template<typename T>
bool const & BlockGeometryStatistics2D<T>::getStatisticsStatus() const
{
  return _statisticsUpdateNeeded;
}


template<typename T>
void BlockGeometryStatistics2D<T>::update(bool verbose)
{
  const_this = const_cast<const BlockGeometryStatistics2D<T>*>(this);

  if (getStatisticsStatus() ) {
    _material2n.clear();
    _blockGeometry->forCoreSpatialLocations([&](auto iX, auto iY) {
      takeStatistics(iX, iY);
    });

    _nMaterials=int();
    std::map<int, int>::iterator iter;
    for (iter = _material2n.begin(); iter != _material2n.end(); iter++) {
      _nMaterials++;
    }

    if (verbose) {
      clout << "updated" << std::endl;
    }
    getStatisticsStatus() = false;
  }
}


template<typename T>
int BlockGeometryStatistics2D<T>::getNmaterials()
{
  update();
  return const_this->getNmaterials();
}

template<typename T>
int BlockGeometryStatistics2D<T>::getNmaterials() const
{
  return _nMaterials;
}

template<typename T>
int BlockGeometryStatistics2D<T>::getNvoxel(int material)
{
  update();
  return const_this->getNvoxel(material);
}

template<typename T>
int BlockGeometryStatistics2D<T>::getNvoxel(int material) const
{
  try {
    return _material2n.at(material);
  }
  catch (std::out_of_range& ex) {
    return 0;
  }
}

template<typename T>
std::map<int, int> BlockGeometryStatistics2D<T>::getMaterial2n()
{
  update();
  return const_this->getMaterial2n();
}

template<typename T>
std::map<int, int> BlockGeometryStatistics2D<T>::getMaterial2n() const
{
  return _material2n;
}

template<typename T>
int BlockGeometryStatistics2D<T>::getNvoxel()
{
  update();
  return const_this->getNvoxel();
}

template<typename T>
int BlockGeometryStatistics2D<T>::getNvoxel() const
{
  int total = 0;
  for (const auto& material : _material2n ) {
    total += material.second;
  }
  return total;
}

template<typename T>
std::vector<int> BlockGeometryStatistics2D<T>::getMinLatticeR(int material)
{
  update();
  return const_this->getMinLatticeR(material);
}

template<typename T>
std::vector<int> BlockGeometryStatistics2D<T>::getMinLatticeR(int material) const
{
  try {
    return _material2min.at(material);
  }
  catch (std::out_of_range& ex) {
    std::vector<int> null;
    return null;
  }
}

template<typename T>
std::vector<int> BlockGeometryStatistics2D<T>::getMaxLatticeR(int material)
{
  update();
  return const_this->getMaxLatticeR(material);
}

template<typename T>
std::vector<int> BlockGeometryStatistics2D<T>::getMaxLatticeR(int material) const
{
  try {
    return _material2max.at(material);
  }
  catch (std::out_of_range& ex) {
    std::vector<int> null;
    return null;
  }
}

template<typename T>
std::vector<T> BlockGeometryStatistics2D<T>::getMinPhysR(int material) const
{
  std::vector<T> tmp(2,T());
  Vector<T,2> physR;
  _blockGeometry->getPhysR(physR, &(getMinLatticeR(material)[0]));
  tmp[0] = physR[0];
  tmp[1] = physR[1];
  return tmp;
}

template<typename T>
std::vector<T> BlockGeometryStatistics2D<T>::getMaxPhysR(int material) const
{
  std::vector<T> tmp(2,T());
  Vector<T,2> physR;
  _blockGeometry->getPhysR(physR, &(getMaxLatticeR(material)[0]));
  tmp[0] = physR[0];
  tmp[1] = physR[1];
  return tmp;
}

template<typename T>
std::vector<T> BlockGeometryStatistics2D<T>::getLatticeExtend(int material)
{
  update();
  return const_this->getLatticeExtend(material);
}

template<typename T>
std::vector<T> BlockGeometryStatistics2D<T>::getLatticeExtend(int material) const
{
  try {
    std::vector<T> extend;
    for (int iDim = 0; iDim < 2; iDim++) {
      extend.push_back(_material2max.at(material)[iDim] - _material2min.at(material)[iDim]);
    }
    return extend;
  }
  catch (std::out_of_range& ex) {
    std::vector<T> null;
    return null;
  }
}

template<typename T>
std::vector<T> BlockGeometryStatistics2D<T>::getPhysExtend(int material)
{
  update();
  return const_this->getPhysExtend(material);
}

template<typename T>
std::vector<T> BlockGeometryStatistics2D<T>::getPhysExtend(int material) const
{
  std::vector<T> extend;
  for (int iDim = 0; iDim < 2; iDim++) {
    extend.push_back(getMaxPhysR(material)[iDim] - getMinPhysR(material)[iDim]);
  }
  return extend;
}

template<typename T>
std::vector<T> BlockGeometryStatistics2D<T>::getPhysRadius(int material)
{
  update();
  return const_this->getPhysRadius(material);
}

template<typename T>
std::vector<T> BlockGeometryStatistics2D<T>::getPhysRadius(int material) const
{
  std::vector<T> radius;
  for (int iDim=0; iDim<2; iDim++) {
    radius.push_back((getMaxPhysR(material)[iDim] - getMinPhysR(material)[iDim])/2.);
  }
  return radius;
}

template<typename T>
std::vector<T> BlockGeometryStatistics2D<T>::getCenterPhysR(int material)
{
  update();
  return const_this->getCenterPhysR(material);
}

template<typename T>
std::vector<T> BlockGeometryStatistics2D<T>::getCenterPhysR(int material) const
{
  std::vector<T> center;
  for (int iDim=0; iDim<2; iDim++) {
    center.push_back(getMinPhysR(material)[iDim] + getPhysRadius(material)[iDim]);
  }
  return center;
}

template<typename T>
std::vector<int> BlockGeometryStatistics2D<T>::getType(const int* input) const
{
  return const_this->getType(input[0], input[1]);
}

template<typename T>
std::vector<int> BlockGeometryStatistics2D<T>::getType(int iX, int iY,
                                                       BlockIndicatorF2D<T>& fluidI,
                                                       BlockIndicatorF2D<T>& outsideI) const
{
  const auto [normalType, normal] = computeBoundaryTypeAndNormal(fluidI, outsideI, {iX,iY});
  return {static_cast<int>(normalType), normal[0], normal[1]};
}

template<typename T>
std::vector<int> BlockGeometryStatistics2D<T>::getType(int iX, int iY) const
{
  BlockIndicatorMaterial2D<T> fluidI(*_blockGeometry, 1);
  BlockIndicatorMaterial2D<T> outsideI(*_blockGeometry, 0);
  return getType(iX,iY,fluidI,outsideI);
}

template<typename T>
std::vector<int> BlockGeometryStatistics2D<T>::computeNormal(int iX, int iY)
{
  return const_this->computeNormal(iX, iY);
}

template<typename T>
std::vector<int> BlockGeometryStatistics2D<T>::computeNormal(int iX, int iY) const
{
  std::vector<int> normal (2,int(0));

  if (iX != 0) {
    if (_blockGeometry->getMaterial({iX - 1, iY}) == 1) {
      normal[0] = -1;
    }
  }
  if (iX != _nX - 1) {
    if (_blockGeometry->getMaterial({iX + 1, iY}) == 1) {
      normal[0] = 1;
    }
  }
  if (iY != 0) {
    if (_blockGeometry->getMaterial({iX, iY - 1}) == 1) {
      normal[1] = -1;
    }
  }
  if (iY != _nY - 1) {
    if (_blockGeometry->getMaterial({iX, iY + 1}) == 1) {
      normal[1] = 1;
    }
  }
  return normal;
}

template<typename T>
std::vector<T> BlockGeometryStatistics2D<T>::computeNormal(int material)
{
  return const_this->computeNormal(material);
}

template<typename T>
std::vector<T> BlockGeometryStatistics2D<T>::computeNormal(int material) const
{
  std::vector<T> normal (2,int(0));
  std::vector<int> minC = getMinLatticeR(material);
  std::vector<int> maxC = getMaxLatticeR(material);
  for (int iX = minC[0]; iX<=maxC[0]; iX++) {
    for (int iY = minC[1]; iY<=maxC[1]; iY++) {
      if (_blockGeometry->getMaterial({iX,iY}) == material) {
        normal[0]+=computeNormal(iX,iY)[0];
        normal[1]+=computeNormal(iX,iY)[1];
      }
    }
  }
  T norm = util::sqrt(normal[0]*normal[0]+normal[1]*normal[1]);
  if (norm>0.) {
    normal[0]/=norm;
    normal[1]/=norm;
  }
  return normal;
}

template<typename T>
std::vector<int> BlockGeometryStatistics2D<T>::computeDiscreteNormal(int material, T maxNorm)
{
  return const_this->computeDiscreteNormal(material, maxNorm);
}

template<typename T>
std::vector<int> BlockGeometryStatistics2D<T>::computeDiscreteNormal(int material, T maxNorm) const
{
  std::vector<T> normal = computeNormal(material);
  std::vector<int> discreteNormal(2,int(0));

  T smallestAngle = T(0);
  for (int iX = -1; iX<=1; iX++) {
    for (int iY = -1; iY<=1; iY++) {
      T norm = util::sqrt(iX*iX+iY*iY);
      if (norm>0.&& norm<maxNorm) {
        T angle = (iX*normal[0] + iY*normal[1])/norm;
        if (angle>=smallestAngle) {
          smallestAngle=angle;
          discreteNormal[0] = iX;
          discreteNormal[1] = iY;
        }
      }
    }
  }
  return discreteNormal;
}

template<typename T>
bool BlockGeometryStatistics2D<T>::check(int material, int iX, int iY,
    unsigned offsetX, unsigned offsetY)
{
  return const_this->check(material, iX, iY, offsetX, offsetY);
}

template<typename T>
bool BlockGeometryStatistics2D<T>::check(int material, int iX, int iY,
    unsigned offsetX, unsigned offsetY) const
{
  bool found = true;
  for (int iOffsetX = -offsetX; iOffsetX <= (int) offsetX; ++iOffsetX) {
    for (int iOffsetY = -offsetY; iOffsetY <= (int) offsetY; ++iOffsetY) {
      if (_blockGeometry->getMaterial({iX + iOffsetX, iY + iOffsetY}) != material) {
        found = false;
      }
    }
  }
  return found;
}

template<typename T>
bool BlockGeometryStatistics2D<T>::find(int material, unsigned offsetX,
                                        unsigned offsetY, int& foundX, int& foundY)
{
  return const_this->find(material, offsetX, offsetY, foundX, foundY);
}

template<typename T>
bool BlockGeometryStatistics2D<T>::find(int material, unsigned offsetX,
                                        unsigned offsetY, int& foundX, int& foundY) const
{
  bool found = false;
  for (foundX = 0; foundX < _nX; foundX++) {
    for (foundY = 0; foundY < _nY; foundY++) {
      found = check(material, foundX, foundY, offsetX, offsetY);
      if (found) {
        return found;
      }
    }
  }
  return found;
}

template<typename T>
void BlockGeometryStatistics2D<T>::print()
{
  update();
  return const_this->print();
}

template<typename T>
void BlockGeometryStatistics2D<T>::print() const
{
  try {
    std::map<int, int>::iterator iter;
    for (const auto& material : _material2n) {
      clout << "materialNumber=" << material.first
            << "; count=" << material.second
            << "; minLatticeR=(" << _material2min.at(material.first)[0] <<","<< _material2min.at(material.first)[1] <<")"
            << "; maxLatticeR=(" << _material2max.at(material.first)[0] <<","<< _material2max.at(material.first)[1] <<")"
            << std::endl;
    }
  }
  catch (std::out_of_range& ex)
  { }
}

template<typename T>
void BlockGeometryStatistics2D<T>::takeStatistics(int iX, int iY)
{

  int type = _blockGeometry->getMaterial({iX, iY});
  if (_material2n.count(type) == 0) {
    _material2n[type] = 1;
    std::vector<int> minCo;
    std::vector<int> maxCo;
    minCo.push_back(iX);
    minCo.push_back(iY);
    _material2min[type] = minCo;
    maxCo.push_back(iX);
    maxCo.push_back(iY);
    _material2max[type] = maxCo;

  }
  else {
    _material2n[type]++;
    if (iX < _material2min[type][0]) {
      _material2min[type][0] = iX;
    }
    if (iY < _material2min[type][1]) {
      _material2min[type][1] = iY;
    }
    if (iX > _material2max[type][0]) {
      _material2max[type][0] = iX;
    }
    if (iY > _material2max[type][1]) {
      _material2max[type][1] = iY;
    }
  }
}

// This function compares two discrete normals (discreteNormal, discreteNormal2) in case of a duplicate assignment of boundary types.
// The goal of this function is to combine these special boundaryVoxels to an existing one (in this case boundary or externalEdge) according to
// the x-, y- and z-values of their discrete normals.
// In the following the algorithm is declared only for the x value, but it is also valid for the y and z values.
//
// for x1 = x2, the new value of x is x1 (1)
// for x1*x2 = -1, the new value of x is 0 (2)
// for x1*x2 = 0, the new value is 0   (3)
//
// It may be possible that all three values equal 0. To avoid that the values are tested again (x²+y²+z²==0) and the loosest assumption (3) is
// redefined to.
//
// If x,y and z == 0 --> find those x,y or z which are 0 because of (3) and redefine them to the value !=0
//
// Additionally the calculated entries are multiplied with (-1) to get the right existing boundary.

} // namespace olb

#endif
