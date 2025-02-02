/*
  Copyright (C) 2005-2009 Steven L. Scott

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

#include <LinAlg/Selector.hpp>
#include <LinAlg/Vector.hpp>
#include <LinAlg/Matrix.hpp>
#include <LinAlg/SpdMatrix.hpp>
#include <LinAlg/Types.hpp>
#include <cpputil/seq.hpp>
#include <cpputil/report_error.hpp>

#include <distributions.hpp>

#include <algorithm>
#include <stdexcept>
#include <sstream>

namespace BOOM {

  namespace {
    std::vector<bool> to_vector_bool(const std::string & s) {
      uint n = s.size();
      std::vector<bool> ans(n,false);
      for (uint i = 0; i < n; ++i) {
        char c = s[i];
        if (c == '1') ans[i] = true;
        else if (c == '0') ans[i] = false;
        else{
          ostringstream err;
          err << "only 0's and 1's are allowed in the 'Selector' "
              << "string constructor "
              << endl
              << "you supplied:  "  << endl
              << s << endl
              << "first illegal value found at position " << i << "." << endl;
          report_error(err.str());
        }
      }
      return ans;
    }
  }  // namespace

  void Selector::reset_included_positions() {
    included_positions_.clear();
    for (uint i = 0; i < nvars_possible(); ++i) {
      if (inc(i)) {
        included_positions_.push_back(i);
      }
    }
  }

  Selector::Selector() {}

  Selector::Selector(uint p, bool all)
      : std::vector<bool>(p,all),
        include_all_(all)
  {
    reset_included_positions();
  }

  Selector::Selector(const std::string &s)
      : std::vector<bool>(to_vector_bool(s)),
        include_all_(false)
  {
    reset_included_positions();
    if (nvars() == nvars_possible()) {
      include_all_ = true;
    }
  }

  Selector::Selector(const std::vector<bool>& in)
      : std::vector<bool>(in),
        include_all_(false)
  {
    reset_included_positions();
  }

  Selector::Selector(const std::vector<uint> &pos, uint n)
      : std::vector<bool>(n, false),
        included_positions_(),
        include_all_(false)
  {
    for (uint i = 0; i < pos.size(); ++i) add(pos[i]);
  }

  void Selector::check_size_eq(uint p, const string &fun) const {
    if (p == nvars_possible()) return;
    ostringstream err;

    err << "error in function Selector::" << fun << endl
        << "Selector::nvars_possible() == " << nvars_possible() << endl
        << "you've assumed it to be " << p << endl;
    report_error(err.str());
  }

  void Selector::check_size_gt(uint p, const string &fun) const {
    if (p< nvars_possible()) return;
    ostringstream err;

    err << "error in function Selector::" << fun << endl
        << "Selector::nvars_possible()== " << nvars_possible() << endl
        << "you tried to access element " << p << endl;
    report_error(err.str());
  }


  Selector & Selector::append(const Selector &rhs) {
    int n0 = nvars_possible();
    std::vector<bool>::resize(size() + rhs.size(), false);
    for (int i = 0; i < rhs.included_positions_.size(); ++i) {
      add(rhs.included_positions_[i] + n0);
    }
    include_all_ *= rhs.include_all_;
    return *this;
  }

  Selector & Selector::append(bool new_last_element) {
    push_back(new_last_element);
    if (new_last_element) {
      included_positions_.push_back(size() - 1);
    }
    include_all_ *= new_last_element;
    return *this;
  }

  Selector & Selector::add(uint p) {
    check_size_gt(p, "add");
    if (include_all_) return *this;
    if (inc(p) == false) {
      (*this)[p] = true;
      std::vector<uint>::iterator it =
        std::lower_bound(included_positions_.begin(),
                         included_positions_.end(),
                         p);
      included_positions_.insert(it, p);}
    return *this;
  }

  void Selector::drop_all() {
    include_all_ = false;
    included_positions_.clear();
    std::vector<bool>::assign(size(), false);
  }

  void Selector::add_all() {
    include_all_ = true;
    uint n = nvars_possible();
    included_positions_ = seq<uint>(0, n-1);
    std::vector<bool>::assign(n, true);
  }

  Selector &Selector::drop(uint p) {
    check_size_gt(p, "drop");
    if (include_all_) {
      reset_included_positions();
      include_all_ = false;
    }
    if (inc(p)) {
      (*this)[p] = false;
      std::vector<uint>::iterator it =
          std::lower_bound(included_positions_.begin(),
                           included_positions_.end(),
                           p);
      included_positions_.erase(it);}
    return *this;
  }

  Selector & Selector::flip(uint p) {
    if (inc(p)) drop(p);
    else add(p);
    return *this;
  }

  Selector Selector::complement() const {
    Selector ans(*this);
    for (uint i = 0; i < nvars_possible(); ++i) {
      ans.flip(i);
    }
    return ans;
  }

  void Selector::swap(Selector &rhs) {
    std::vector<bool>::swap(rhs);
    std::swap(included_positions_, rhs.included_positions_);
    std::swap(include_all_, rhs.include_all_);
  }

  bool Selector::inc(uint i) const { return (*this)[i];}

  uint Selector::nvars() const {
    return include_all_ ? nvars_possible() : included_positions_.size(); }

  uint Selector::nvars_possible() const {return size();}

  uint Selector::nvars_excluded() const {return nvars_possible() - nvars();}

  uint Selector::indx(uint i) const {
    if (include_all_) return i;
    return included_positions_[i]; }
  uint Selector::INDX(uint i) const {
    if (include_all_) return i;
    std::vector<uint>::const_iterator loc =
      std::lower_bound(included_positions_.begin(),
                       included_positions_.end(),
                       i);
    return loc - included_positions_.begin();
  }

  Vector Selector::to_Vector() const {
    Vector ans(nvars_possible(), 0.0);
    uint n = nvars();
    for (uint i = 0; i < n; ++i) {
      uint I = indx(i);
      ans[I] = 1;
    }
    return ans;
  }

  bool Selector::covers(const Selector &rhs) const {
    for (uint i = 0; i < rhs.nvars(); ++i) {
      uint I = rhs.indx(i);
      if (!inc(I)) return false;}
    return true;}

  bool Selector::operator==(const Selector &rhs) const {
    const std::vector<bool> & RHS(rhs);
    const std::vector<bool> & LHS(*this);
    return LHS == RHS;
  }

  bool Selector::operator!=(const Selector &rhs) const {
    return ! operator==(rhs);
  }

  Selector Selector::Union(const Selector &rhs) const {
    Selector ans(*this);
    ans += rhs;
    return ans;
  }

  Selector Selector::intersection(const Selector &rhs) const {
    Selector ans(*this);
    ans *= rhs;
    return ans;
  }

  // Returns a Selector of the same size as this, which includes all
  // the elements where *this and that differ.  *this and that must be
  // the same size.
  Selector Selector::exclusive_or(const Selector &that) const {
    uint n = nvars_possible();
    check_size_eq(that.nvars_possible(), "exclusive_or");
    Selector ans(n, false);
    for (int i = 0; i < n; ++i) {
      ans[i] = (*this)[i] != that[i];
    }
    return ans;
  }

  Selector & Selector::cover(const Selector &rhs) {
    check_size_eq(rhs.nvars_possible(), "cover");
    for (uint i = 0; i < rhs.nvars(); ++i)
      add(rhs.indx(i));  // does nothing if already added
    return *this;
  }

  template <class V>
  Vector inc_select(const V &x, const Selector &inc) {
    uint nx = x.size();
    uint N = inc.nvars_possible();
    if (nx != N) {
      ostringstream msg;
      msg << "Selector::select... x.size() = " << nx << " nvars_possible() = "
          << N << endl;
      report_error(msg.str());
    }
    uint n = inc.nvars();

    if (n == N) return x;
    Vector ans(n);
    for (uint i = 0; i < n; ++i) ans[i] = x[inc.indx(i)];
    return ans;
  }

  Vector Selector::select(const Vector &x) const {
    return inc_select<Vector>(x, *this); }
  Vector Selector::select(const VectorView &x) const {
    return inc_select<VectorView>(x, *this); }
  Vector Selector::select(const ConstVectorView &x) const {
    return inc_select<ConstVectorView>(x, *this); }

  template <class V>
  Vector inc_expand(const V &x, const Selector &inc) {
    uint n = inc.nvars();
    uint nx = x.size();
    if (nx!=n) {
      ostringstream msg;
      msg << "Selector::expand... x.size() = " << nx << " nvars() = "
          << n << endl;
      report_error(msg.str());
    }
    uint N = inc.nvars_possible();
    if (n == N) return x;
    Vector ans(N, 0);
    for (uint i = 0; i < n; ++i) {
      uint I = inc.indx(i);
      ans[I] = x[i];
    }
    return ans;
  }

  Vector Selector::expand(const Vector & x) const {
    return inc_expand(x,*this); }
  Vector Selector::expand(const VectorView & x) const {
    return inc_expand(x,*this); }
  Vector Selector::expand(const ConstVectorView & x) const {
    return inc_expand(x,*this); }

  Vector Selector::select_add_int(const Vector &x) const {
    assert(x.size() == nvars_possible()-1);
    if (include_all_) return concat(1.0,x);
    Vector ans(nvars());
    ans[0]= inc(0) ? 1.0 : x[indx(0)-1];
    for (uint i = 1; i < nvars(); ++i) ans[i] = x[indx(i)-1];
    return ans;
  }
  //----------------------------------------------------------------------
  SpdMatrix Selector::select(const SpdMatrix &S) const {
    uint n = nvars();
    uint N = nvars_possible();
    check_size_eq(S.ncol(), "select");
    if (include_all_ || n == N) return S;
    SpdMatrix ans(n);
    for (uint i = 0; i < n; ++i) {
      uint I = included_positions_[i];
      const double * s(S.col(I).data());
      double * a(ans.col(i).data());
      for (uint j = 0; j < n; ++j) {
        a[j] = s[included_positions_[j]];
      }
    }
    return ans;
  }
  //----------------------------------------------------------------------
  Matrix Selector::select_cols(const Matrix &m) const {
    if (include_all_) return m;
    Matrix ans(m.nrow(), nvars());
    for (uint i = 0; i < nvars(); ++i) {
      uint I = indx(i);
      std::copy(m.col_begin(I), m.col_end(I), ans.col_begin(i));
    }
    return ans;
  }

  Matrix Selector::select_rows(const Matrix &m) const {
    if (include_all_) return m;
    uint n = nvars();
    Matrix ans(n, m.ncol());
    for (uint i = 0; i < n; ++i) ans.row(i) = m.row(indx(i));
    return ans;
  }

  Matrix Selector::select_square(const Matrix &m) const {
    assert(m.is_square());
    check_size_eq(m.nrow(), "select_square");
    if (include_all_) return m;

    Matrix ans(nvars(), nvars());
    for (uint i = 0; i < nvars(); ++i) {
      uint I = indx(i);
      for (uint j = 0; j < nvars(); ++j) {
        uint J = indx(j);
        ans(i, j) = m(I, J); }}
    return ans;
  }

  Vector & Selector::zero_missing_elements(Vector &v) const {
    uint N = nvars_possible();
    check_size_eq(v.size(), "zero_missing_elements");
    const Selector & inc(*this);
    for (uint i = 0; i < N; ++i) {
      if (!inc[i]) v[i] = 0;
    }
    return v;
  }

  void Selector::sparse_multiply(
      const Matrix &m, const Vector &v, VectorView ans) const {
    bool m_already_sparse = ncol(m) == nvars();
    if (!m_already_sparse) {
      check_size_eq(m.ncol(), "sparse_multiply");
    }
    bool v_already_sparse = v.size() == nvars();
    if (!v_already_sparse) {
      check_size_eq(v.size(), "sparse_multiply");
    }
    ans = 0;

    for (int i = 0; i < included_positions_.size(); ++i) {
      uint I = included_positions_[i];
      ans.axpy(m.col(m_already_sparse ? i : I),
               v[v_already_sparse ? i : I]);
    }
  }

  Vector Selector::sparse_multiply(
      const Matrix &m, const Vector &v) const {
    Vector ans(m.nrow(), 0.0);
    this->sparse_multiply(m, v, VectorView(ans));
    return ans;
  }

  Vector Selector::sparse_multiply(
      const Matrix &m, const VectorView &v) const {
    Vector ans(m.nrow(), 0.0);
    this->sparse_multiply(m, v, VectorView(ans));
    return ans;
  }

  Vector Selector::sparse_multiply(
      const Matrix &m, const ConstVectorView &v) const {
    Vector ans(m.nrow(), 0.0);
    this->sparse_multiply(m, v, VectorView(ans));
    return ans;
  }


  namespace {
    template <class VEC1, class VEC2>
    double do_sparse_dot_product(const Selector &inc,
                                 const VEC1 &full,
                                 const VEC2 &sparse) {
      int n = inc.nvars_possible();
      if (full.size() != n || sparse.size() > n) {
        report_error("Vector sizes incompatible in sparse dot product.");
      }
      double ans = 0;
      for (int i = 0; i < inc.nvars(); ++i) {
        ans += sparse[i] * full[inc.indx(i)];
      }
      return ans;
    }
  }  // namespace

  double Selector::sparse_dot_product(const Vector &full,
                                      const Vector &sparse) const {
    return do_sparse_dot_product(*this, full, sparse);
  }
  double Selector::sparse_dot_product(const Vector &full,
                                      const VectorView &sparse) const {
    return do_sparse_dot_product(*this, full, sparse);
  }
  double Selector::sparse_dot_product(const Vector &full,
                                      const ConstVectorView &sparse) const {
    return do_sparse_dot_product(*this, full, sparse);
  }
  double Selector::sparse_dot_product(const VectorView &full,
                                      const Vector &sparse) const {
    return do_sparse_dot_product(*this, full, sparse);
  }
  double Selector::sparse_dot_product(const VectorView &full,
                                      const VectorView &sparse) const {
    return do_sparse_dot_product(*this, full, sparse);
  }
  double Selector::sparse_dot_product(const VectorView &full,
                                      const ConstVectorView &sparse) const {
    return do_sparse_dot_product(*this, full, sparse);
  }
  double Selector::sparse_dot_product(const ConstVectorView &full,
                                      const Vector &sparse) const {
    return do_sparse_dot_product(*this, full, sparse);
  }
  double Selector::sparse_dot_product(const ConstVectorView &full,
                                      const VectorView &sparse) const {
    return do_sparse_dot_product(*this, full, sparse);
  }
  double Selector::sparse_dot_product(const ConstVectorView &full,
                                      const ConstVectorView &sparse) const {
    return do_sparse_dot_product(*this, full, sparse);
  }

  int Selector::random_included_position(RNG &rng) const {
    int number_included = nvars();
    if (number_included == 0) {
      return -1;
    }
    int j = random_int_mt(rng, 0, number_included - 1);
    return indx(j);
  }

  int Selector::random_excluded_position(RNG &rng) const {
    int N = nvars_possible();
    int n = nvars();
    int number_excluded = N - n;
    if (number_excluded == 0) return -1;
    if ((static_cast<double>(number_excluded) / N) >= .5) {
      // If the number of excluded variables is a large fraction of
      // the total then perform the random selection by rejection sampling.
      while (true) {
        int j = random_int_mt(rng, 1, N-1);
        if (!inc(j)) return j;
      }
    } else {
      int which_excluded_variable = random_int_mt(rng, 1, number_excluded);
      int number_excluded_so_far = 0;
      for (int i = 0; i < N; ++i) {
        if (!inc(i)) {
          ++number_excluded_so_far;
          if (number_excluded_so_far == which_excluded_variable) {
            return i;
          }
        }
      }
    }
    return -1;
  }

  void Selector::push_back(bool element) {
    std::vector<bool>::push_back(element);
    if (element) {
      included_positions_.push_back(size() - 1);
    } else {
      include_all_ = false;
    }
  }

  void Selector::erase(uint which_element) {
    bool included = inc(which_element);
    std::vector<bool>::erase(this->begin() + which_element);
    if (included) {
      auto it = std::lower_bound(included_positions_.begin(),
                                 included_positions_.end(),
                                 which_element);
      if (*it != which_element) {
        report_error("Error erasing element from selector.");
      }
      included_positions_.erase(it);
    } else {
      // If the element that was erased was the only element excluded
      // then we might need to flip the include_all_ bit.
      if (nvars() == nvars_possible()) {
        include_all_ = true;
      }
    }
  }

  Selector &Selector::operator+=(const Selector &rhs) {
    return cover(rhs);}

  Selector & Selector::operator*=(const Selector &rhs) {
    Selector tmp = intersection(rhs);
    this->swap(tmp);
    return *this;
  }

  ostream & operator<<(ostream &out, const Selector &inc) {
    for (uint i = 0; i < inc.nvars_possible(); ++i) out << inc.inc(i);
    return out;
  }

  istream & operator>>(istream &in, Selector &inc) {
    string s;
    in >> s;
    uint n = s.size();
    std::vector<bool> tmp(n);
    for (uint i = 0; i < n; ++i) {
      if (s[i] == '0') tmp[i] = false;
      else if (s[i] == '1') tmp[i] = true;
      else report_error(s + "is an illegal input value for 'Selector'");
    }
    Selector blah(tmp);
    inc.swap(blah);
    return in;
  }

  //============================================================

  inline bool check_vec(const Vector &big, int pos, const Vector &small) {
    for (uint i = 0; i < small.size(); ++i) {
      uint I = i;
      if (I >= big.size()) return false;
      if (big[pos + I] != small[i]) return false;
    }
    return true;
  }

  Selector find_contiguous_subset(const Vector &big, const Vector &small) {
    std::vector<bool> vec(big.size(), false);
    Vector::const_iterator b = big.begin();
    Vector::const_iterator it = big.begin();
    Vector::const_iterator end = big.end();

    for (uint i = 0; i < small.size(); ++i) {
      it = std::find(it, end, small[i]);
      uint I = it-b;
      vec[I] = true;
    }
    return Selector(vec);
  }

}  // namespace BOOM
