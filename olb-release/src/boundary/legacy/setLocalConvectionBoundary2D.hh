/*  This file is part of the OpenLB library
 *
 *  Copyright (C) 2020 Alexander Schulz
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

//This file contains the Local Convection Boundary
//This is a new version of the Boundary, which only contains free floating functions
#ifndef SET_LOCAL_CONVECTION_BOUNDARY_2D_HH
#define SET_LOCAL_CONVECTION_BOUNDARY_2D_HH

#include "setLocalConvectionBoundary2D.h"

namespace olb {

///Initialising the LocalConvectionBoundary on the superLattice domain
template<typename T, typename DESCRIPTOR>
void setLocalConvectionBoundary(SuperLattice<T, DESCRIPTOR>& sLattice, SuperGeometry<T,2>& superGeometry, int material, T* uAv)
{
  setLocalConvectionBoundary<T,DESCRIPTOR>(sLattice, superGeometry.getMaterialIndicator(material), uAv);
}
///Initialising the LocalConvectionBoundary on the superLattice domain
template<typename T, typename DESCRIPTOR>
void setLocalConvectionBoundary(SuperLattice<T, DESCRIPTOR>& sLattice, FunctorPtr<SuperIndicatorF2D<T>>&& indicator, T* uAv)
{
  int _overlap = 0; // TODO: This intended?
  for (int iCloc = 0; iCloc < sLattice.getLoadBalancer().size(); ++iCloc) {
    setLocalConvectionBoundary<T,DESCRIPTOR>(sLattice.getBlock(iCloc),
        indicator->getBlockIndicatorF(iCloc), uAv);
  }
  /// Adds needed Cells to the Communicator _commBC in SuperLattice
  //TODO: Is communication really needed for this BC?
  addPoints2CommBC<T, DESCRIPTOR>(sLattice, std::forward<decltype(indicator)>(indicator), _overlap);
}

///Set LocalConvectionBoundary for indicated cells inside the block domain
template<typename T, typename DESCRIPTOR>
void setLocalConvectionBoundary(BlockLattice<T,DESCRIPTOR>& block, BlockIndicatorF2D<T>& indicator, T* uAv)
{
  OstreamManager clout (std::cout, "setLocalConvectionBoundary");
  bool _output = false;
  auto& blockGeometryStructure = indicator.getBlockGeometry();
  const int margin = 1;
  std::vector<int> discreteNormal(3, 0);
  blockGeometryStructure.forSpatialLocations([&](auto iX, auto iY) {
    if (blockGeometryStructure.getNeighborhoodRadius({iX, iY}) >= margin
        && indicator(iX, iY)) {
      PostProcessorGenerator2D<T, DESCRIPTOR>* postProcessor = nullptr;
      discreteNormal = indicator.getBlockGeometry().getStatistics().getType(iX, iY);
      if (discreteNormal[0] == 0) {//set postProcessors on indicated LocalConvectionBoundary cells
        if (discreteNormal[1] == -1) {
          if (_output) {
            clout << "setLocalConvectionBoundary<" << 0 << ","<< -1 << ">("  << iX << ", "<< iX << ", " << iY << ", " << iY << " )" << std::endl;
          }
          postProcessor = nullptr;
        }
        else if (discreteNormal[1] == 1) {
          if (_output) {
            clout << "setLocalConvectionBoundary<" << 0 << ","<< 1 << ">("  << iX << ", "<< iX << ", " << iY << ", " << iY << " )" << std::endl;
          }
          postProcessor = nullptr;
        }
        else if (discreteNormal[2] == -1) {
          if (_output) {
            clout << "setLocalConvectionBoundary<" << 1 << ","<< -1 << ">("  << iX << ", "<< iX << ", " << iY << ", " << iY << " )" << std::endl;
          }
          postProcessor = nullptr;
        }
        else if (discreteNormal[2] == 1) {
          if (_output) {
            clout << "setLocalConvectionBoundary<" << 1 << ","<< 1 << ">("  << iX << ", "<< iX << ", " << iY << ", " << iY << " )" << std::endl;
          }
          postProcessor = nullptr;
        }
        if (postProcessor) {
          block.addPostProcessor(*postProcessor);
        }
      }
    }
  });
}

}//namespace olb


#endif
