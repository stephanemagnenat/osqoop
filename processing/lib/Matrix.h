/*
    liban - base function to do usefull things related to simulation and scientific work
    Copyright (C) 1999-2005 Stephane Magnenat <stephane at magnenat dot net>
    Copyright (c) 2004-2005 Antoine Beyeler <antoine dot beyeler at epfl dot ch>
    Copyright (C) 2005 Laboratory of Intelligent Systems, EPFL, Lausanne
    See AUTHORS for details

    This program is free software. You can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __MATRIX_H
#define __MATRIX_H

#include <valarray>
#include <numeric>
#include <cassert>

/*!	\file Matrix.h
	\brief Stroustrup approved n x m templatised Matrix implementation
*/

namespace Teem
{
	template<typename T> class SliceIter;
	
	//! Return true if p == q
	//! \ingroup utils
	template<typename T> bool operator==(const SliceIter<T>& p, const SliceIter<T>& q)
	{
		return p.curr == q.curr && p.s.stride() == q.s.stride() && p.s.start() == q.s.start();
	}
	
	//! Return true if p != q
	//! \ingroup utils
	template<typename T> bool operator!=(const SliceIter<T>& p, const SliceIter<T>& q)
	{
		return !(p==q);
	}
	
	//! Return true if p < q
	//! \ingroup utils
	template<typename T> bool operator<(const SliceIter<T>& p, const SliceIter<T>& q)
	{
		return p.curr < q.curr && p.s.stride() == q.s.stride() && p.s.start() == q.s.start();
	}
	
	//! Stroustrup approved slice iterator class.
	//! \ingroup utils
	template<typename T> class SliceIter
	{
	public:
		//! Construct an iterator on array vv using slide ss
		SliceIter(std::valarray<T> *vv, const std::slice &ss) : v(vv), s(ss), curr(0) { }
		
		//! Return iterator to the end of slice
		SliceIter end() const
		{
			SliceIter t = *this;
			t.curr = s.size();
			return t;
		}
		
		//! Increment current position
		SliceIter& operator++() { curr++; return *this; }
		//! Increment current position
		SliceIter operator++(int) { SliceIter t = *this; curr++; return t; }
		
		//! access element i
		T& operator[](size_t i) { return ref(i); }
		//! access element i
		T& operator()(size_t i) { return ref(i); }
		//! access element at current position
		T& operator*() { return ref(curr); }
		
		//! Return true if p == q
		friend bool operator== <>(const SliceIter& p, const SliceIter& q);
		//! Return true if p != q
		friend bool operator!= <>(const SliceIter& p, const SliceIter& q);
		//! Return true if p < q
		friend bool operator< <>(const SliceIter& p, const SliceIter& q);
		
	protected:
		std::valarray<T> *v; //!< array containing the datas
		const std::slice s; //!< slice for iteration
		size_t curr; //!< current position
		
		//! access element i
		T& ref(size_t i) const { return (*v)[s.start() + i * s.stride()]; }
	};
	
	
	template<typename T> class ConstSliceIter;
	
	//! Return true if p == q
	//! \ingroup utils
	template<typename T> bool operator==(const ConstSliceIter<T>& p, const ConstSliceIter<T>& q)
	{
		return p.curr == q.curr && p.s.stride() == q.s.stride() && p.s.start() == q.s.start();
	}
	
	//! Return true if p != q
	//! \ingroup utils
	template<typename T> bool operator!=(const ConstSliceIter<T>& p, const ConstSliceIter<T>& q)
	{
		return !(p==q);
	}
	
	//! Return true if p < q
	//! \ingroup utils
	template<typename T> bool operator<(const ConstSliceIter<T>& p, const ConstSliceIter<T>& q)
	{
		return p.curr < q.curr && p.s.stride() == q.s.stride() && p.s.start() == q.s.start();
	}
	
	//! Stroustrup approved slice iterator class for const matrix.
	//! \ingroup utils
	template<typename T> class ConstSliceIter
	{
	public:
		//! Construct a const iterator on array vv using slide ss
		ConstSliceIter(const std::valarray<T> *vv, const std::slice &ss) : v(vv), s(ss), curr(0) { }
		
		//! Return iterator to the end of slice
		ConstSliceIter end() const
		{
			ConstSliceIter t = *this;
			t.curr = s.size();
			return t;
		}
		
		//! Increment current position
		ConstSliceIter& operator++() { curr++; return *this; }
		//! Increment current position
		ConstSliceIter operator++(int) { ConstSliceIter t = *this; curr++; return t; }
		
		//! access element i
		const T& operator[](size_t i) const { return ref(i); }
		//! access element i
		const T& operator()(size_t i) const { return ref(i); }
		//! access element at current position
		const T& operator*() const { return ref(curr); }
		
		//! Return true if p == q
		friend bool operator== <>(const ConstSliceIter& p, const ConstSliceIter& q);
		//! Return true if p != q
		friend bool operator!= <>(const ConstSliceIter& p, const ConstSliceIter& q);
		//! Return true if p < q
		friend bool operator< <>(const ConstSliceIter& p, const ConstSliceIter& q);
		
	protected:
		const std::valarray<T> *v; //!< array containing the datas
		const std::slice s; //!< slice for iteration
		size_t curr; //!< current position
		
		//! access element i
		const T& ref(size_t i) const { return (*v)[s.start() + i * s.stride()]; }
	};
	
	//! Stroustrup approved matrix class.
	//! \ingroup utils
	template<typename T> class Matrix
	{
	public:
		//! Create a matrix of size nx x my, initialise all elements with c
		Matrix(size_t nx = 0, size_t ny = 0, T c = T()) : items(c, nx*ny), xDim(nx), yDim(ny) { }
		//! Destroy matrix and its elements
		~Matrix() { }
		
		//! Return the number of element
		size_t size() const { return xDim * yDim; }
		//! Return the number of columns
		size_t columnNum() const { return xDim; }
		//! Return the number of rows
		size_t rowNum() const { return yDim; }
		
		//! Return a reference to the underlying std::valarray container. This can be used to apply operators on all items.
		std::valarray<T>& flat() { return items; }
		
		//! Return a const reference to the underlying std::valarray container
		const std::valarray<T>& const_flat() const { return items; }
		
		//! Set matrix size to nx x ny, initialise all elements with c
		void resize(size_t nx, size_t ny, T c = T()) { items.resize(nx*ny, c); xDim = nx; yDim = ny; }
		
		//! Return row i
		SliceIter<T> row(size_t i) { return SliceIter<T>(&items, std::slice(i, xDim, yDim)); }
		//! Return a constant version of row i
		ConstSliceIter<T> row(size_t i) const { return ConstSliceIter<T>(&items, std::slice(i, xDim, yDim)); }
		
		//! Return column i
		SliceIter<T> column(size_t i) { return SliceIter<T>(&items, std::slice(i*yDim, yDim, 1)); }
		//! Return a constant version of column i
		ConstSliceIter<T> column(size_t i) const { return ConstSliceIter<T>(&items, std::slice(i*yDim, yDim, 1)); }
		
		//! Return element at pos (x, y)
		T& operator() (size_t x, size_t y) { return column(x)[y]; }
		//! Return a constant version of element at pos (x, y)
		const T& operator() (size_t x, size_t y) const { return column(x)[y]; }
		
		//! Return column i
		SliceIter<T> operator() (size_t i) { return column(i); }
		//! Return a constant version of column i
		ConstSliceIter<T> operator() (size_t i) const { return column(i); }
		
	protected:
		std::valarray<T> items; //!< matrix content
		size_t xDim; //!< matrix width
		size_t yDim; //!< matrix height
	};
	
	//! Performs a inner product. Used for the Mv (matrix-vector) multiplication.
	//! \ingroup utils
	template<typename T> T operator*(const ConstSliceIter<T> &v1, const std::valarray<T> &v2)
	{
		T res = 0;
		for(size_t i = 0; i < v2.size(); i++)
			res += v1[i] * v2[i];
		return res;
	}
	
	//! Performs multiplication between a matrix and a vector.
	//! @param m Matrix to use. Column num must be equal to size of v.
	//! @param v Vector to use.
	//! @return Vector resulting from the multiplication. Size is the same than row num of m
	//! \ingroup utils
	template<typename T> std::valarray<T> operator*(const Matrix<T>& m, const std::valarray<T>& v)
	{
		assert(m.columnNum() == v.size());
		
		std::valarray<T> res(m.rowNum());
		for(size_t i = 0; i < m.rowNum(); i++)
			res[i] = m.row(i) * v;
		return res;
	}
	
	//! Performs multiplication between a vector and a matrix v*M.
	//! @param v Row vector to use.
	//! @param m Matrix to use. Row num must be equal to size of v.
	//! @return Result of multplication. Size is same than col num of m.
	//! \ingroup utils
	template<typename T> std::valarray<T> operator*(const std::valarray<T>& v, const Matrix<T>& m)
	{
		assert(m.rowNum() == v.size());
		
		std::valarray<T> res(m.columnNum());
		for(size_t i = 0; i < m.columnNum(); i++)
			res[i] = m.column(i) * v;
		return res;
	}
}

#endif
