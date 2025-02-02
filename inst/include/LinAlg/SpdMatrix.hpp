/*
  Copyright (C) 2005 Steven L. Scott

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

#ifndef NEW_LA_SPD_MATRIX_H
#define NEW_LA_SPD_MATRIX_H
#include <LinAlg/Matrix.hpp>

namespace BOOM{

  class SpdMatrix
    : public Matrix
  {
    // symmetric, positive definite mattrix with 'square' storage
    // (i.e. 0's are stored)
  public:
    SpdMatrix();
    SpdMatrix(uint dim, double diag=0.0);
    SpdMatrix(uint dim, double *m, bool ColMajor=true);
    template <class FwdIt>
    explicit SpdMatrix(FwdIt Beg, FwdIt End);
    SpdMatrix(const SpdMatrix &sm);  // reference semantics

    // Args:
    //   m: A Matrix object that happens to be symmetric and positive
    //     definite.
    //   check: If true, then throw an exception if m is not
    //     symmetric.  Skip the check if 'check' is false.
    SpdMatrix(const Matrix &m, bool check = true);
    SpdMatrix(const SubMatrix &m, bool check = true);
    SpdMatrix(const ConstSubMatrix &m, bool check = true);

    SpdMatrix & operator=(const SpdMatrix &); // value semantics
    SpdMatrix & operator=(const Matrix &);
    SpdMatrix & operator=(const SubMatrix &);
    SpdMatrix & operator=(const ConstSubMatrix &);
    SpdMatrix & operator=(double x);
    bool operator==(const SpdMatrix &)const;

    void  swap(SpdMatrix &rhs);
    void randomize() override;  // fills entries with U(0,1) random variables,
                       // then multiply by self-transpose.

    //-------- size and shape info ----------
    virtual uint nelem()const;         // number of distinct elements
    uint dim()const{return nrow();}

    //--------- change size and shape ----------
    SpdMatrix & resize(uint n);

    // -------- row and column operations ----------
    SpdMatrix & set_diag(double x, bool zero_offdiag=true);
    SpdMatrix & set_diag(const Vector &v, bool zero_offdiag=true);

    //------------- Linear Algebra -----------
    //      lower_triangular_Matrix chol() const;
    Matrix chol() const;
    Matrix chol(bool & ok) const;
    SpdMatrix inv()const;
    SpdMatrix inv(bool &ok)const;
    double det() const override;
    double logdet() const;
    double logdet(bool &ok) const;

    // Returns this^{-1} * mat.  Throws an exception if this cannot be
    // inverted.
    Matrix solve(const Matrix &mat) const override;

    // Returns this^{-1} * v and sets ok to true.  If this cannot be
    // inverted ok is set to false and the return value a Vector of
    // the same dimension as rhs filled with negative_infinity.
    Vector solve(const Vector &v, bool &ok) const;

    // Returns this{-1} * v.  Throws an exception if this cannot be
    // inverted.
    Vector solve(const Vector &v) const override;

    void reflect();   // copies upper triangle into lower triangle

    // Returns the Mahalinobis distance:  (x - y)^T (*this) (x - y).
    double Mdist(const Vector &x, const Vector &y) const ;

    // Mahalinobis distance from 0:  x^T (*this) x
    double Mdist(const Vector &x) const ;

    SpdMatrix & add_outer(const Vector &x, double w = 1.0,
      		    bool force_sym=true);     // *this+= w*x*x^T
    SpdMatrix & add_outer(const VectorView &x, double w = 1.0,
      		    bool force_sym=true);     // *this+= w*x*x^T
    SpdMatrix & add_outer(const ConstVectorView &x, double w = 1.0,
      		    bool force_sym=true);     // *this+= w*x*x^T
    SpdMatrix & add_outer(const Matrix &X, double w=1.0,
                          bool force_sym = true);   // *this+= w*X*X^T

    SpdMatrix & add_outer_w(const Vector &x, double w = 1.0){
      return add_outer(x,w); }

    SpdMatrix & add_inner(const Matrix &x, double w=1.0);
    SpdMatrix & add_inner(const Matrix &X, const Vector & w,
      		    bool force_sym=true);  // *this+= X^T w X

    // *this  += w x.t()*y + y.t()*x;
    SpdMatrix & add_inner2(const Matrix &x, const Matrix &y, double w=1.0);
    // *this  += w x*y.t() + y*x.t();
    SpdMatrix & add_outer2(const Matrix &x, const Matrix &y, double w=1.0);

    SpdMatrix & add_outer2(const Vector &x, const Vector &y, double w = 1.0);

    //--------- Matrix multiplication ------------
    Matrix & mult(const Matrix &B, Matrix &ans, double scal=1.0)const override;
    Matrix & Tmult(const Matrix &B, Matrix &ans, double scal=1.0)const override;
    Matrix & multT(const Matrix &B, Matrix &ans, double scal=1.0)const override;

    Matrix & mult(const SpdMatrix &B, Matrix &ans, double scal=1.0)const override;
    Matrix & Tmult(const SpdMatrix &B, Matrix &ans, double scal=1.0)const override;
    Matrix & multT(const SpdMatrix &B, Matrix &ans, double scal=1.0)const override;

    Matrix & mult(const DiagonalMatrix &B, Matrix &ans, double scal=1.0)const override;
    Matrix & Tmult(const DiagonalMatrix &B, Matrix &ans, double scal=1.0)const override;
    Matrix & multT(const DiagonalMatrix &B, Matrix &ans, double scal=1.0)const override;

    Vector & mult(const Vector &v, Vector &ans, double scal=1.0)const override;
    Vector & Tmult(const Vector &v, Vector &ans, double scal=1.0)const override;

    //------------- input/output ---------------
    virtual Vector vectorize(bool minimal=true)const;
    virtual void unvectorize(const Vector &v, bool minimal=true);
    Vector::const_iterator unvectorize(Vector::const_iterator &b,
      				 bool minimal=true);
    void make_symmetric(bool have_upper_triangle=true);
  };

  typedef SpdMatrix Spd;

  //______________________________________________________________________
  template <class Fwd>
  SpdMatrix::SpdMatrix(Fwd b, Fwd e){
    uint n = std::distance(b,e);
    uint m = lround( ::sqrt(static_cast<double>(n)));
    assert(m*m == n);
    resize(m);
    std::copy(b,e,begin());
  }

  SpdMatrix operator*(double x, const SpdMatrix &V);
  SpdMatrix operator*(const SpdMatrix &v, double x);
  SpdMatrix operator/(const SpdMatrix &v, double x);

  SpdMatrix Id(uint p);

  SpdMatrix RTR(const Matrix &R, double a = 1.0); // a * R^T%*%R
  SpdMatrix LLT(const Matrix &L, double a = 1.0); // a * L%*%L^T

  SpdMatrix outer(const Vector &v);
  SpdMatrix outer(const VectorView &v);
  SpdMatrix outer(const ConstVectorView &v);

  Matrix chol(const SpdMatrix &Sigma);
  Matrix chol(const SpdMatrix &Sigma, bool &ok);

  inline double logdet(const SpdMatrix & Sigma){ return Sigma.logdet();}

  SpdMatrix chol2inv(const Matrix &L);
  // Returns A^{-1}, where L is the cholesky factor of A.

  SpdMatrix sandwich(const Matrix &A, const SpdMatrix &V); // AVA^t
  SpdMatrix sandwich_old(const Matrix &A, const SpdMatrix &V); // AVA^t

  SpdMatrix select(const SpdMatrix &X, const std::vector<bool> &inc,
      	      uint nvars);
  SpdMatrix select(const SpdMatrix &X, const std::vector<bool> &inc);
  SpdMatrix as_symmetric(const Matrix &A);

  SpdMatrix sum_self_transpose(const Matrix &A);  // A + A.t()

  // Returns the vector of eigenvalues of X, sorted from smallest to
  // largest.
  Vector eigenvalues(const SpdMatrix &X);

  // Args:
  //   V:  The matrix to decompose.
  //   eigenvectors:  On return the columns of 'eigenvectors' are the
  //     eigenvectors coresponding to the eigenvalues in the same
  //     position.
  // Returns: the vector of eigenvalues of V, sorted from smallest to
  //   largest.
  //
  // The relationship is V = Q^T Lambda Q, or Q * V = Lambda * Q,
  // where Q^T = eigenvectors.
  Vector eigen(const SpdMatrix &V, Matrix & eigenvectors);

  // Returns the largest eigenvalue of X.
  double largest_eigenvalue(const SpdMatrix &X);

  // An SpdMatrix X can be written X = Q^T Lambda Q, where the columns
  // of Q^T contain the eigenvectors (i.e. the eigenvectors are the
  // rows of Q), and Lambda is a diagonal matrix containing the
  // eigenvalues.
  //
  // The symmetric square root of X is Q^T Lambda^{1/2} Q.
  SpdMatrix symmetric_square_root(const SpdMatrix &X);

  // An SpdMatrix X can be written X = Q^T Lambda Q, where the columns
  // of Q^T contain the eigenvectors (i.e. the eigenvectors are the
  // rows of Q), and Lambda is a diagonal matrix containing the
  // eigenvalues.  The "eigen_root" is a matrix square root of X
  // defined as Z = Lambda^{1/2} * Q.  It is a matrix square root in
  // the sense that Z^T * Z = Q^T * Lambda^{1/2} * Lambda^{1/2} * Q =
  // X.
  //
  // Note that the eigen_root can be multiplied by any orthogonal
  // matrix A to produce W = A * Lambda^{1/2} * Q, which preserves the
  // relationship W^T * W = X.
  Matrix eigen_root(const SpdMatrix &X);
}
#endif // NEW_LA_SPD_MATRIX_H
