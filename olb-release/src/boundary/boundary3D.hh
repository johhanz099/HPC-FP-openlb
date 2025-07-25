/*  This file is part of the OpenLB library
 *
 *  Copyright (C) 2007 the OpenLB project
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
 * Groups all the generic 3D template files in the boundaryConditions directory.
 */
#include "boundary/postprocessor/advectionDiffusionBoundaryPostProcessor3D.hh"
#include "boundaryPostProcessors3D.hh"
#include "extendedFiniteDifferenceBoundary3D.hh"
#include "offBoundaryPostProcessors3D.hh"
#include "localPressure3D.h"
#include "setZeroGradientBoundary3D.hh"
#include "legacy/defineU3D.hh"
#include "slip3D.hh"
#include "partialSlip3D.hh"
#include "legacy/setWallFunctionBoundary3D.hh"
#include "legacy/wallFunctionBoundaryPostProcessors3D.hh"
#include "legacy/setBouzidiVelocityBoundary3D.hh"
#include "legacy/setBouzidiZeroVelocityBoundary3D.hh"
