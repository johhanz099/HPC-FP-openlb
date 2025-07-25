/*  This file is part of the OpenLB library
 *
 *  Copyright (C) 2007 Jonas Latt
 *                2024 Dennis Teutscher
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
 * Groups all the 3D include files in the directory utilities.
 */

#include "aDiff.h"
#include "adHelpers.h"
#include "benchmarkUtil.h"
#include "calc.h"
#include "norm.h"
#include "timer.h"
#include "vectorHelpers.h"
#include "functorPtr.h"
#include "functorDsl3D.h"
#include "hyperplane3D.h"
#include "hyperplaneLattice3D.h"
#include "anisoDiscr.h"
#include "omath.h"
#include "oalgorithm.h"
#include "random.h"
#include "typeIndexedContainers.h"
#include "integrationTestUtils.h"
#include "matrix.h"
#include "geometricOperations.h"
#include "permeability.h"
#include "line3D.h"
#include "lineLattice3D.h"
#ifdef FEATURE_PROJ
#include "osmParser.h"
#endif
