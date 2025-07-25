/*  This file is part of the OpenLB library
 *
 *  Copyright (C) 2012-2018 Lukas Baron, Tim Dornieden, Mathias J. Krause,
 *  Albert Mink, Adrian Kummerlaender
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

#ifndef INTERPOLATION_F_2D_H
#define INTERPOLATION_F_2D_H

#include "analyticalF.h"
#include "functors/lattice/blockBaseF2D.h"
#include "functors/lattice/superBaseF2D.h"
#include "geometry/cuboidDecomposition.h"
#include "geometry/blockGeometry.h"
#include "geometry/superGeometry.h"

namespace olb {

/// Converts block functors to analytical functors (special version for 2D)
template <typename T, typename W = T>
class SpecialAnalyticalFfromBlockF2D final : public AnalyticalF2D<T,W> {
protected:
  BlockF2D<W>& _f;
  Cuboid2D<T>& _cuboid;
  Vector<T,2> _delta;
  T _scale;
public:
  SpecialAnalyticalFfromBlockF2D(BlockF2D<W>& f, Cuboid2D<T>& cuboid, Vector<T,2> delta, T scale = 1.);
  bool operator() (W output[], const T physC[]) override;
};

/// Converts block functors to analytical functors
template <typename T, typename W = T>
class AnalyticalFfromBlockF2D final : public AnalyticalF2D<T,W> {
protected:
  BlockF2D<W>& _f;
  Cuboid2D<T>& _cuboid;
public:
  AnalyticalFfromBlockF2D(BlockF2D<W>& f, Cuboid2D<T>& cuboid);
  bool operator() (W output[], const T physC[]) override;
};

/// Converts super functions to analytical functions
template <typename T, typename W = T>
class AnalyticalFfromSuperF2D final : public AnalyticalF2D<T,W> {
protected:
  const bool _communicateToAll;
  const bool _communicateOverlap;

  SuperF2D<T>& _f;
  CuboidDecomposition<T,2>& _cuboidDecomposition;

  std::vector<std::unique_ptr<AnalyticalFfromBlockF2D<T,W>>> _blockF;
public:
  AnalyticalFfromSuperF2D(SuperF2D<T>& f,
                          bool communicateToAll=false,
                          bool communicateOverlap=true);
  bool operator() (T output[], const T physC[]) override;

  /// \return Size of _blockF vector
  int getBlockFSize() const;
  /// \return _blockF[iCloc]
  AnalyticalFfromBlockF2D<T,W>& getBlockF(int iCloc);
};


} // end namespace olb

#endif
