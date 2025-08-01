/*  This file is part of the OpenLB library
 *
 *  Copyright (C) 2012 Lukas Baron, Mathias J. Krause, Albert Mink
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

#ifndef BLOCK_BASE_F_3D_H
#define BLOCK_BASE_F_3D_H

#include "functors/genericF.h"
#include "core/blockData.h"
#include "core/blockStructure.h"
#include "core/unitConverter.h"

#include <memory>

/* Note: Throughout the whole source code directory genericFunctions, the
 *  template parameters for i/o dimensions are:
 *           F: S^m -> T^n  (S=source, T=target)
 */

namespace olb {


template <typename T> class BlockIndicatorF3D;

/// represents all functors that operate on a cuboid in general, mother class of BlockLatticeF, ..
template <typename T>
class BlockF3D : public GenericF<T,int> {
protected:
  BlockF3D(BlockStructureD<3>& blockStructure, int targetDim);
  BlockStructureD<3>& _blockStructure;
public:
  /// virtual destructor for defined behaviour
  ~BlockF3D() override {};
  virtual BlockStructureD<3>& getBlockStructure() const;

  using GenericF<T,int>::operator();

  BlockF3D<T>& operator-(BlockF3D<T>& rhs);
  BlockF3D<T>& operator+(BlockF3D<T>& rhs);
  BlockF3D<T>& operator*(BlockF3D<T>& rhs);
  BlockF3D<T>& operator/(BlockF3D<T>& rhs);
};

/// BlockDataF3D can store data of any BlockFunctor3D
template <typename T, typename BaseType>
class BlockDataF3D : public BlockF3D<BaseType> {
protected:
  BlockDataF3D(int nx, int ny, int nz, int size=1);

  std::unique_ptr<BlockData<3,T,BaseType>> _blockDataStorage;
  BlockData<3,T,BaseType>&                 _blockData;
public:
  /// Constructor
  BlockDataF3D(BlockData<3,T,BaseType>& blockData);
  /// to store functor data, constuctor creates _blockData with functor data
  BlockDataF3D(BlockF3D<BaseType>& f);
  /// returns _blockData
  BlockData<3,T,BaseType>& getBlockData();
  /// access to _blockData via its get()
  bool operator() (BaseType output[], const int input[]) override;
};

/// identity functor
template <typename T>
class BlockIdentity3D final : public BlockF3D<T> {
protected:
  BlockF3D<T>& _f;
public:
  BlockIdentity3D(BlockF3D<T>& f);
  // access operator should not delete f, since f still has the identity as child
  bool operator() (T output[], const int input[]) override;
};

/// functor to extract one component
template <typename T>
class BlockExtractComponentF3D : public BlockF3D<T> {
protected:
  BlockF3D<T>& _f;
  int _extractDim;
public:
  BlockExtractComponentF3D(BlockF3D<T>& f, int extractDim);
  int getExtractDim();
  bool operator() (T output[], const int input[]);
};

/// functor to extract one component inside an indicator
template <typename T>
class BlockExtractComponentIndicatorF3D : public BlockExtractComponentF3D<T> {
protected:
  BlockIndicatorF3D<T>& _indicatorF;
public:
  BlockExtractComponentIndicatorF3D(BlockF3D<T>& f, int extractDim,
                                    BlockIndicatorF3D<T>& indicatorF);
  bool operator() (T output[], const int input[]) override;
};

/// functor to extract data inside an indicator
template <typename T>
class BlockExtractIndicatorF3D : public BlockF3D<T> {
protected:
  BlockF3D<T>&          _f;
  BlockIndicatorF3D<T>& _indicatorF;
public:
  BlockExtractIndicatorF3D(BlockF3D<T>& f,
                           BlockIndicatorF3D<T>& indicatorF);
  bool operator() (T output[], const int input[]);
};

/// perform explicit typecast from output type T2 to T
// user has to guarantee that cast is well-defined
template <typename T, typename T2>
class BlockTypecastF3D : public BlockF3D<T> {
protected:
  BlockF3D<T2>&          _f;
public:
  BlockTypecastF3D(BlockF3D<T2>& f);
  bool operator() (T output[], const int input[]);
};

/// represents all functors that operate on a DESCRIPTOR in general, e.g. getVelocity(), getForce(), getPressure()
template <typename T, typename DESCRIPTOR>
class BlockLatticeF3D : public BlockF3D<T> {
protected:
  BlockLatticeF3D(BlockLattice<T,DESCRIPTOR>& blockLattice, int targetDim);
  BlockLattice<T,DESCRIPTOR>& _blockLattice;
public:
  /// Copy Constructor
  //BlockLatticeF3D(BlockLatticeF3D<T,DESCRIPTOR> const& rhs);
  /// Assignment Operator
  //BlockLatticeF3D<T,DESCRIPTOR>& operator=(BlockLatticeF3D<T,DESCRIPTOR> const& rhs);

  BlockLattice<T,DESCRIPTOR>& getBlock();
};


/// identity functor
template <typename T, typename DESCRIPTOR>
class BlockLatticeIdentity3D final : public BlockLatticeF3D<T,DESCRIPTOR> {
protected:
  BlockLatticeF3D<T,DESCRIPTOR>& _f;
public:
  BlockLatticeIdentity3D(BlockLatticeF3D<T,DESCRIPTOR>& f);
  bool operator() (T output[], const int input[]) override;
};


/// represents all functors that operate on a DESCRIPTOR with output in Phys, e.g. physVelocity(), physForce(), physPressure()
template <typename T, typename DESCRIPTOR>
class BlockLatticePhysF3D : public BlockLatticeF3D<T,DESCRIPTOR> {
protected:
  const UnitConverter<T,DESCRIPTOR>& _converter;
  BlockLatticePhysF3D(BlockLattice<T,DESCRIPTOR>& blockLattice,
                      const UnitConverter<T,DESCRIPTOR>& converter, int targetDim);
public:
  /// Copy Constructor
  //BlockLatticePhysF3D(BlockLatticePhysF3D<T,DESCRIPTOR> const& rhs);
  /// Assignment Operator
  //BlockLatticePhysF3D<T,DESCRIPTOR>& operator=(BlockLatticePhysF3D<T,DESCRIPTOR> const& rhs);
};

/// represents all thermal functors that operate on a DESCRIPTOR with output in Phys, e.g. physTemperature(), physHeatFlux()
template <typename T, typename DESCRIPTOR, typename TDESCRIPTOR>
class BlockLatticeThermalPhysF3D : public BlockLatticeF3D<T,TDESCRIPTOR> {
protected:
  BlockLatticeThermalPhysF3D(BlockLattice<T,TDESCRIPTOR>& blockLattice,
                             const ThermalUnitConverter<T,DESCRIPTOR,TDESCRIPTOR>& converter, int targetDim);
  const ThermalUnitConverter<T,DESCRIPTOR,TDESCRIPTOR>& _converter;
};

template <typename T, typename W = T>
class BlockConst3D final : public BlockF3D<W> {
protected:
  const std::vector<W> _c;
public:
  /// Constructor accepting std::vector
  /**
   * \param blockStructure Only required to instantiate underlying BlockF3D
   * \param v              std::vector to be copied to output by operator.
   *                       Size determines target dimension.
   **/
  BlockConst3D(BlockStructureD<3>& blockStructure, std::vector<W> v);

  /// Constructor accepting single scalar
  BlockConst3D(BlockStructureD<3>& blockStructure, W scalar);
  /// Constructor template accepting vectors
  template <unsigned Size>
  BlockConst3D(BlockStructureD<3>& blockStructure, Vector<W,Size> v)
    : BlockConst3D(blockStructure, v.toStdVector()) { };

  bool operator() (W output[], const int input[]) override;
};


/// @brief Generate a BlockLatticeF from an arbitrary function
/// @tparam T floating point type
template <typename T, typename DESCRIPTOR>
class BlockLatticeFfromCallableF final : public BlockLatticeF<T,DESCRIPTOR> {
private:
  // local block number
  const unsigned _iC;
  // internal function with interface defined by BlockF:
  // takes local coordinates, writes result into array, returns bool
  std::function<bool(T*,const int*)> _f;

public:
  /// @brief Generate a BlockLatticeF from an arbitrary function
  /// @tparam F function type
  /// @param iC local cuboid number
  /// @param f some function, e.g. lambda expression
  /// The following types of f are accepted:
  /// void(T* output, auto cell)
  /// void(T* output, auto cell, int iC, const int* localCoordinates)
  /// void(T* output, int iC, const int* localCoordinates)
  template <typename F>
  BlockLatticeFfromCallableF(BlockLattice<T,DESCRIPTOR>& blockLattice, unsigned iC, F&& f)
   : BlockLatticeF<T,DESCRIPTOR>(blockLattice, 1), _iC(iC),
    _f([&,f](T* output, const int* input) -> bool {
      auto cell = blockLattice.get(input);
      if constexpr (std::is_invocable_v<F, T*, decltype(cell)>) {
        f(output, cell);
      } else if constexpr (std::is_invocable_v<F, T*, decltype(cell), int, const int*>) {
        f(output, cell, _iC, input);
      } else {
        f(output, _iC, input);
      }
      return true;
    })
  { }

  bool operator() (T output[], const int input[]) override {
    return _f(output, input);
  }
};


} // end namespace olb

#endif
