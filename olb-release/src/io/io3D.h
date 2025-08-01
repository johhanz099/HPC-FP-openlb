/*  This file is part of the OpenLB library
 *
 *  Copyright (C) 2007 Jonas Latt
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
 * Groups all the 3D include files for the directory io.
 */
#include "printUtils.h"
#include "base64.h"
#include "blockGifWriter.h"
#include "blockVtkWriter3D.h"
#include "colormaps.h"
#include "fileName.h"
#include "gnuplotHeatMapWriter.h"
#include "gnuplotWriter.h"
#include "ostreamManager.h"
#include "parallelIO.h"
#include "serializerIO.h"
#include "stlReader.h"
#include "superVtmWriter3D.h"
#include "vtiReader.h"
#include "vtiWriter.h"
#include "xmlReader.h"
#include "cliReader.h"
#include "octree.h"
#include "vtkWriter.h"
#include "vtkSurfaceWriter.h"
#include "consoleWriter.h"
#include "blockLatticeSTLreader.h"
#include "plainWriter.h"
#include "analyticalPorosityVolumeF.h"
#include "analyticalVelocityVolumeF.h"
#include "consoleToFileWriter.h"
