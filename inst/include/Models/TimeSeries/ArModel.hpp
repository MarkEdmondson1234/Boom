/*
  Copyright (C) 2005-2012 Steven L. Scott

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

#ifndef BOOM_AR_MODEL_HPP_
#define BOOM_AR_MODEL_HPP_

#include <Models/Policies/ParamPolicy_2.hpp>
#include <Models/Policies/SufstatDataPolicy.hpp>
#include <Models/Policies/PriorPolicy.hpp>
#include <Models/Glm/RegressionModel.hpp>
#include <Models/TimeSeries/TimeSeriesSufstatDataPolicy.hpp>
#include <Models/TimeSeries/TimeSeries.hpp>
#include <deque>

namespace BOOM {

  // ArSuf keeps track of the sufficient statistics for an AR(p)
  // model, which is a regression of y[t] on y[t-1] ... y[t-p].  Each
  // time the class is shown a data point to update it creates a new
  // 'RegressionData' containing the current lags of the series as 'X'
  // and the current value as 'y'.
  class ArSuf : public SufstatDetails<DoubleData> {
   public:
    ArSuf(int number_of_lags);
    ArSuf * clone() const override;
    void clear() override;

    void Update(const DoubleData &y) override;
    void add_mixture_data(double y, const Vector &lags, double weight);

    ArSuf * abstract_combine(Sufstat *s) override;
    void combine(Ptr<ArSuf> s);
    void combine(const ArSuf &s);
    Vector vectorize(bool minimal = true)const override;
    Vector::const_iterator unvectorize(Vector::const_iterator &v,
                                            bool minimal = true) override;
    Vector::const_iterator unvectorize(const Vector &v,
                                            bool minimal = true) override;
    ostream & print(ostream &out)const override;

    // forwarded calls to reg_suf_...
    double n()const{return reg_suf_->n();}
    double yty()const{return reg_suf_->yty();}
    Vector xty()const{return reg_suf_->xty();}
    SpdMatrix xtx()const{return reg_suf_->xtx();}

   private:
    // lags must be in the same order as the AR coefficients
    Ptr<NeRegSuf> reg_suf_;

    std::deque<double> lags_;
    Vector x_;
  };

  // An AR(p) model for the time series y[t], defined by
  //
  // y[t] = \sum_{i = 1}^p phi[i] * y[t-i] + epsilon[t]
  //
  // with epsilon[t] \sim N(0, sigma^2).
  //
  // The parameters of this model are the vector of autoregression
  // coefficients phi[1]...phi[p], and the innovation variance
  // sigma^2.
  class ArModel
      : public ParamPolicy_2<VectorParams, UnivParams>,
        public SufstatDataPolicy<DoubleData, ArSuf>,
        public PriorPolicy
  {
   public:
    ArModel(int number_of_lags = 1);
    ArModel(Ptr<VectorParams> autoregression_coefficients,
            Ptr<UnivParams> innovation_variance);
    ArModel * clone()const override;

    int number_of_lags()const;

    double sigma()const;
    double sigsq()const;
    const Vector & phi()const;

    void set_sigma(double sigma);
    void set_sigsq(double sigsq);
    void set_phi(const Vector &phi);

    Ptr<VectorParams> Phi_prm();
    const Ptr<VectorParams> Phi_prm()const;
    Ptr<UnivParams> Sigsq_prm();
    const Ptr<UnivParams> Sigsq_prm()const;

    // Returns a vector giving the autocovariance of the model for 0,
    // 1, 2, ..., number_of_lags lags.
    Vector autocovariance(int number_of_lags)const;

    // Returns true if the polynomial \phi(z)
    //
    // 1 - phi[0]*z - phi[1]*z^2 - ... - phi[p-1] z^p
    //
    // has all its (complex) roots outside the unit circle, which is a
    // requirement for an AR(p) process to be stationary.
    static bool check_stationary(const Vector &phi);

    // Simulate n time points from the process, starting from the
    // stationary distribution.
    Vector simulate(int n)const;

    // Simulate n time points from the process, starting from y0 as an
    // initial condition.
    Vector simulate(int n, const Vector &y0)const;

   private:
    // An AR(p) process can be represented as a white noise filter:
    // y[t] = \sum_{i = 0}^\infty \psi[i] Z_{t-i}, where Z_t is IID
    // N(0, \sigma^2).  The coefficients "psi" in this filter can be
    // obtained by the polynomial inversion \psi(z) = 1 / \phi(z).
    // This inversion can be done by equating coefficients in the
    // equation \phi(z) \psi(z) = 1.  This implies \psi[0] = 1 and
    // generates a recurrence relationship for all higher order
    // coefficients (which must sum to zero).  The "\psi" coefficients
    // are stored as filter_coefficients_.
    mutable Vector filter_coefficients_;
    mutable bool filter_coefficients_current_;

    void set_filter_coefficients()const;
    void observe_phi() {
      filter_coefficients_current_ = false;
    }
  };

}  // namespace BOOM
#endif  //  BOOM_AR_MODEL_HPP_
