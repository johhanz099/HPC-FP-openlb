/*  This file is part of the OpenLB library
 *
 *  Copyright (C) 2021-24 Adrian Kummerlaender, Shota Ito
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

/*  ========================================================
 *  ==  WARNING: This is an automatically generated file, ==
 *  ==                  do not modify.                    ==
 *  ========================================================
 */

#pragma once


namespace olb {

namespace dynamics {

template <typename T, typename... FIELDS>
struct CSE<dynamics::Tuple<T, descriptors::D2Q9<FIELDS...>, momenta::Tuple<momenta::VelocityBoundaryDensity<0, -1>, momenta::FixedVelocityMomentumGeneric, momenta::BulkStress, momenta::DefineSeparately>, equilibria::SecondOrder, collision::BGK, dynamics::DefaultCombination>> {
template <concepts::Cell CELL, concepts::Parameters PARAMETERS, concepts::BaseType V=typename CELL::value_t>
CellStatistic<V> collide(CELL& cell, PARAMETERS& parameters) any_platform {
auto x9 = cell.template getFieldComponent<momenta::FixedVelocityMomentumGeneric::VELOCITY>(0);
auto x10 = cell.template getFieldComponent<momenta::FixedVelocityMomentumGeneric::VELOCITY>(1);
auto x11 = parameters.template get<descriptors::OMEGA>();
auto x12 = x11 + V{-1};
auto x13 = x9 + V{-1};
auto x14 = cell[0] + V{2}*cell[1] + V{2}*cell[2] + V{2}*cell[3] + cell[4] + cell[8] + V{1};
auto x15 = -x14/x13;
auto x16 = x10*x10;
auto x17 = V{1.5}*x16;
auto x18 = x9*x9;
auto x19 = V{1.5}*x18;
auto x20 = x17 + x19 + V{-1};
auto x21 = V{0.0277777777777778}*x11;
auto x22 = V{3}*x9;
auto x23 = -x22;
auto x24 = x10 - x9;
auto x25 = V{1} - x17;
auto x26 = -x19;
auto x27 = V{3}*x10;
auto x28 = x26 + x27;
auto x29 = x25 + x28;
auto x30 = V{0.111111111111111}*x11;
auto x31 = V{3}*x18 + x25;
auto x32 = x10 + x9;
auto x33 = x32*x32;
auto x34 = x26 - x27;
auto x35 = V{3}*x16 + V{1};
auto x36 = -x24;
auto x0 = -cell[0]*x12 - V{0.444444444444444}*x11*(x15*x20 + V{1});
auto x1 = -(cell[1]*x12 + x21*(-x15*(x23 + x29 + V{4.5}*(x24*x24)) + V{1}));
auto x2 = -cell[2]*x12 - x30*(-x15*(x23 + x31) + V{1});
auto x3 = -cell[3]*x12 - x21*(-x15*(-x20 - x22 - x27 + V{4.5}*x33) + V{1});
auto x4 = -cell[4]*x12 - x30*(-x15*(x34 + x35) + V{1});
auto x5 = -(cell[5]*x12 + x21*(-x15*(x22 + x25 + x34 + V{4.5}*(x36*x36)) + V{1}));
auto x6 = -cell[6]*x12 - x30*(-x15*(x22 + x31) + V{1});
auto x7 = -cell[7]*x12 - x21*(-x15*(x22 + x29 + V{4.5}*x33) + V{1});
auto x8 = -cell[8]*x12 - x30*(-x15*(x28 + x35) + V{1});
cell[0] = x0;
cell[1] = x1;
cell[2] = x2;
cell[3] = x3;
cell[4] = x4;
cell[5] = x5;
cell[6] = x6;
cell[7] = x7;
cell[8] = x8;
return { -V{1}*x14/x13, x16 + x18 };
}
};

}

}
