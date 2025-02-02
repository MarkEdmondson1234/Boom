/*
  Copyright (C) 2007-2011 Steven L. Scott

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

#ifndef BOOM_ARRAY_HPP
#define BOOM_ARRAY_HPP

#include <LinAlg/Vector.hpp>
#include <LinAlg/VectorView.hpp>
#include <LinAlg/Matrix.hpp>
#include <LinAlg/SpdMatrix.hpp>
#include <LinAlg/ArrayIterator.hpp>

#include <vector>
#include <cpputil/report_error.hpp>

namespace BOOM{
  // ConstArrayBase implements the const methods common to the Array
  // class and its views.  It implements all size-related queries, as
  // well as indexing and equality comparisons.

  class ConstArrayBase{
   public:
    ConstArrayBase();
    ConstArrayBase(const std::vector<int> &dims);
    ConstArrayBase(const std::vector<int> &dims,
                   const std::vector<int> &strides);
    virtual ~ConstArrayBase(){}

    virtual const double *data()const=0;

    double operator[](const std::vector<int> &index)const;

    int ndim()const{return dims_.size();}
    int dim(int i)const{return dims_[i];}
    const std::vector<int> &dim()const{return dims_;}

    // stride(i) is the number of steps you must advance in data()
    // to increment the i'th index by one.
    int stride(int i)const{return strides_[i];}
    const std::vector<int> &strides()const{return strides_;}

    // size() is the number of elements stored in the array.  It is
    // the product of dims_;
    int size()const;

    // If an Array is the same size and shape as another Array-like
    // thing then they can be compared with operator==
    bool operator==(const Vector &rhs)const;
    bool operator==(const VectorView &rhs)const;
    bool operator==(const ConstVectorView &rhs)const;
    // Matrix & allows for SpdMatrix comparisons as well
    bool operator==(const Matrix &rhs)const;
    bool operator==(const ConstArrayBase &rhs)const;

    // operator() is supported for up to six arguments.  An
    // exception will be thrown if the number of arguments supplied
    // does not match the dimension of dims_ and strides_.
    double operator()(int x1)const;
    double operator()(int x1, int x2)const;
    double operator()(int x1, int x2, int x3)const;
    double operator()(int x1, int x2, int x3, int x4)const;
    double operator()(int x1, int x2, int x3, int x4, int x5)const;
    double operator()(int x1, int x2, int x3, int x4, int x5, int x6)const;

    // Utillity functions for creating a std::vector<int> to be used
    // as an index.  Up to 6 dimensions are supported.  More can be
    // added if needed, but if arrays of greater than 6 dimensions
    // are needed, then people will probably create the dimensions
    // programmatically.
    static std::vector<int> index1(int x1);
    static std::vector<int> index2(int x1, int x2);
    static std::vector<int> index3(int x1, int x2, int x3);
    static std::vector<int> index4(int x1, int x2, int x3, int x4);
    static std::vector<int> index5(int x1, int x2, int x3, int x4, int x5);
    static std::vector<int> index6(
        int x1, int x2, int x3, int x4, int x5, int x6);

   private:
    std::vector<int> dims_;
    std::vector<int> strides_;
   protected:
    void reset_dims(const std::vector<int> &dims);
    void reset_strides(const std::vector<int> &strides);
    void compute_strides();
    static int product(const std::vector<int> &dims);
  };
  //======================================================================
  class ArrayBase : public ConstArrayBase {
   public:
    typedef ArrayIterator iterator;

    ArrayBase();
    ArrayBase(const std::vector<int> &dims);
    ArrayBase(const std::vector<int> &dims, const std::vector<int> &strides);

    using ConstArrayBase::data;
    virtual double *data()=0;

    using ConstArrayBase::operator[];
    double & operator[](const std::vector<int> &index);

    using ConstArrayBase::operator();
    double & operator()(int x1);
    double & operator()(int x1, int x2);
    double & operator()(int x1, int x2, int x3);
    double & operator()(int x1, int x2, int x3, int x4);
    double & operator()(int x1, int x2, int x3, int x4, int x5);
    double & operator()(int x1, int x2, int x3, int x4, int x5, int x6);

  };

  //======================================================================
  inline bool operator==(const Vector &lhs, const ConstArrayBase &rhs){
    return rhs==lhs;}
  inline bool operator==(const VectorView &lhs, const ConstArrayBase &rhs){
    return rhs==lhs;}
  inline bool operator==(const ConstVectorView &lhs, const ConstArrayBase &rhs){
    return rhs==lhs;}
  inline bool operator==(const Matrix &lhs, const ConstArrayBase &rhs){
    return rhs==lhs;}
  //======================================================================
  class Array;
  class ConstArrayView : public ConstArrayBase {
   public:
    typedef ConstArrayIterator const_iterator;
    typedef ConstArrayIterator iterator;

    ConstArrayView(const Array &);
    ConstArrayView(const double *data, const std::vector<int> &dims);
    ConstArrayView(const double *data,
                   const std::vector<int> &dims,
                   const std::vector<int> &strides);
    ConstArrayView(const ConstArrayBase &rhs);

    const double *data()const override{return data_;}

    void reset(const double *data,
               const std::vector<int> &dims);
    void reset(const double *data,
               const std::vector<int> &dims,
               const std::vector<int> &strides);

    // 'slice' returns a lower dimensional view into an array.  If you
    // have a 3-way array indexed by (i, j, k), and you want to get
    // the (i, k) slice (that is, (i, 0, k), (i, 1, k), ...), then you
    // call array.slice(i, -1, k).  The negative index says 'give me
    // all of these', analogous to a missing index in R.  The return
    // value is a view into the array with dimension equal to the
    // number of negative arguments.
    ConstArrayView slice(const std::vector<int> &index)const;
    ConstArrayView slice(int x1)const;
    ConstArrayView slice(int x1, int x2)const;
    ConstArrayView slice(int x1, int x2, int x3)const;
    ConstArrayView slice(int x1, int x2, int x3, int x4)const;
    ConstArrayView slice(int x1, int x2, int x3, int x4, int x5)const;
    ConstArrayView slice(int x1, int x2, int x3, int x4, int x5, int x6)const;

    // vector_slice() works in exactly the same way as slice(), but it
    // returns a VectorView instead of an ArrayView.  Exactly one
    // index must be negative.
    ConstVectorView vector_slice(const std::vector<int> &index)const;
    ConstVectorView vector_slice(int x1)const;
    ConstVectorView vector_slice(int x1, int x2)const;
    ConstVectorView vector_slice(int x1, int x2, int x3)const;
    ConstVectorView vector_slice(int x1, int x2, int x3, int x4)const;
    ConstVectorView vector_slice(int x1, int x2, int x3, int x4, int x5)const;
    ConstVectorView vector_slice(
        int x1, int x2, int x3, int x4, int x5, int x6)const;

    ConstArrayIterator begin()const;
    ConstArrayIterator end()const;
   private:
    const double *data_;
  };
  //======================================================================
  class ArrayView : public ArrayBase {
   public:
    typedef ConstArrayIterator const_iterator;
    typedef ArrayIterator iterator;

    ArrayView(Array &);
    ArrayView(double *data, const std::vector<int> &dims);
    ArrayView(double *data,
              const std::vector<int> &dims,
              const std::vector<int> &strides);

          double *data() override{return data_;}
    const double *data()const override{return data_;}

    void reset(double *data, const std::vector<int> &dims);
    void reset(double *data,
               const std::vector<int> &dims,
               const std::vector<int> &strides);

    ArrayView & operator=(const Array &a);
    ArrayView & operator=(const ArrayView &a);
    ArrayView & operator=(const ConstArrayView &a);
    ArrayView & operator=(const Matrix &a);
    ArrayView & operator=(const Vector &a);
    ArrayView & operator=(const VectorView &a);
    ArrayView & operator=(const ConstVectorView &a);

    // 'slice' returns a lower dimensional view into an array.  If you
    // have a 3-way array indexed by (i, j, k), and you want to get
    // the (i, k) slice (that is, (i, 0, k), (i, 1, k), ...), then you
    // call array.slice(i, -1, k).  The negative index says 'give me
    // all of these', analogous to a missing index in R.  The return
    // value is a view into the array with dimension equal to the
    // number of negative arguments.
    ConstArrayView slice(const std::vector<int> &index)const;
    ConstArrayView slice(int x1)const;
    ConstArrayView slice(int x1, int x2)const;
    ConstArrayView slice(int x1, int x2, int x3)const;
    ConstArrayView slice(int x1, int x2, int x3, int x4)const;
    ConstArrayView slice(int x1, int x2, int x3, int x4, int x5)const;
    ConstArrayView slice(int x1, int x2, int x3, int x4, int x5, int x6)const;

    ArrayView slice(const std::vector<int> &index);
    ArrayView slice(int x1);
    ArrayView slice(int x1, int x2);
    ArrayView slice(int x1, int x2, int x3);
    ArrayView slice(int x1, int x2, int x3, int x4);
    ArrayView slice(int x1, int x2, int x3, int x4, int x5);
    ArrayView slice(int x1, int x2, int x3, int x4, int x5, int x6);

    // vector_slice() works in exactly the same way as slice(), but it
    // returns a VectorView instead of an ArrayView.  Exactly one
    // index must be negative.
    VectorView vector_slice(const std::vector<int> &index);
    VectorView vector_slice(int x1);
    VectorView vector_slice(int x1, int x2);
    VectorView vector_slice(int x1, int x2, int x3);
    VectorView vector_slice(int x1, int x2, int x3, int x4);
    VectorView vector_slice(int x1, int x2, int x3, int x4, int x5);
    VectorView vector_slice(
        int x1, int x2, int x3, int x4, int x5, int x6);

    ConstVectorView vector_slice(const std::vector<int> &index)const;
    ConstVectorView vector_slice(int x1)const;
    ConstVectorView vector_slice(int x1, int x2)const;
    ConstVectorView vector_slice(int x1, int x2, int x3)const;
    ConstVectorView vector_slice(int x1, int x2, int x3, int x4)const;
    ConstVectorView vector_slice(int x1, int x2, int x3, int x4, int x5)const;
    ConstVectorView vector_slice(
        int x1, int x2, int x3, int x4, int x5, int x6)const;

    ConstArrayIterator begin()const;
    ConstArrayIterator end()const;
    ArrayIterator begin();
    ArrayIterator end();
   private:
    double *data_;
  };

  //======================================================================
  class Array : public ArrayBase{
   public:
    typedef std::vector<double>::iterator iterator;
    typedef std::vector<double>::const_iterator const_iterator;

    // Sets data to zero
    Array(){}
    Array(const std::vector<int> &dims, double initial_value = 0);
    Array(const std::vector<int> &dims, const std::vector<double> &data);

    // Fill the array with U(0,1) random numbers
    void randomize();

    Array & operator=(const Array &rhs);

    template <class FwdIt>
    Array & assign(FwdIt begin, FwdIt end){
      data_.assign(begin, end);
      if(data_.size() != this->size()){
        report_error("Wrong sized data passed to Array::assign");
      }
      return *this;
    }

    double * data() override{return data_.data();}
    const double * data()const override{return data_.data();}

    using ConstArrayBase::operator==;
    bool operator==(const Array &rhs)const;

    // 'slice' returns a lower dimensional view into an array.  If you
    // have a 3-way array indexed by (i, j, k), and you want to get
    // the (i, k) slice (that is, (i, 0, k), (i, 1, k), ...), then you
    // call array.slice(i, -1, k).  The negative index says 'give me
    // all of these', analogous to a missing index in R.  The return
    // value is a view into the array with dimension equal to the
    // number of negative arguments.
    ConstArrayView slice(const std::vector<int> &index)const;
    ConstArrayView slice(int x1)const;
    ConstArrayView slice(int x1, int x2)const;
    ConstArrayView slice(int x1, int x2, int x3)const;
    ConstArrayView slice(int x1, int x2, int x3, int x4)const;
    ConstArrayView slice(int x1, int x2, int x3, int x4, int x5)const;
    ConstArrayView slice(int x1, int x2, int x3, int x4, int x5, int x6)const;

    ArrayView slice(const std::vector<int> &index);
    ArrayView slice(int x1);
    ArrayView slice(int x1, int x2);
    ArrayView slice(int x1, int x2, int x3);
    ArrayView slice(int x1, int x2, int x3, int x4);
    ArrayView slice(int x1, int x2, int x3, int x4, int x5);
    ArrayView slice(int x1, int x2, int x3, int x4, int x5, int x6);

    // vector_slice() works in exactly the same way as slice(), but it
    // returns a ConstVectorView instead of a ConstArrayView.  Exactly
    // one index must be negative.
    ConstVectorView vector_slice(const std::vector<int> &index)const;
    ConstVectorView vector_slice(int x1)const;
    ConstVectorView vector_slice(int x1, int x2)const;
    ConstVectorView vector_slice(int x1, int x2, int x3)const;
    ConstVectorView vector_slice(int x1, int x2, int x3, int x4)const;
    ConstVectorView vector_slice(int x1, int x2, int x3, int x4, int x5)const;
    ConstVectorView vector_slice(
        int x1, int x2, int x3, int x4, int x5, int x6)const;

    VectorView vector_slice(const std::vector<int> &index);
    VectorView vector_slice(int x1);
    VectorView vector_slice(int x1, int x2);
    VectorView vector_slice(int x1, int x2, int x3);
    VectorView vector_slice(int x1, int x2, int x3, int x4);
    VectorView vector_slice(int x1, int x2, int x3, int x4, int x5);
    VectorView vector_slice(int x1, int x2, int x3, int x4, int x5, int x6);

    iterator begin(){return data_.begin();}
    iterator end(){return data_.end();}
    const_iterator begin()const{return data_.begin();}
    const_iterator end()const{return data_.end();}
   private:
    std::vector<double> data_;
  };
}
#endif// BOOM_ARRAY_HPP
