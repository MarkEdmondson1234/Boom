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

#include <Models/StateSpace/StateModels/ArStateModel.hpp>
#include <distributions.hpp>
#include <cpputil/report_error.hpp>
#include <boost/bind.hpp>

namespace BOOM {

  ArStateModel::ArStateModel(int number_of_lags)
      : ArModel(number_of_lags),
        state_transition_matrix_(
            new AutoRegressionTransitionMatrix(Phi_prm())),
        state_variance_matrix_(
            new UpperLeftCornerMatrix(number_of_lags, 1.0)),
        state_variance_is_current_(false),
        observation_matrix_(number_of_lags),
        initial_state_mean_(number_of_lags, 0.0),
        initial_state_variance_(number_of_lags, 1.0),
        stationary_initial_distribution_(false)
  {
    observation_matrix_[0] = 1.0;
    DataPolicy::only_keep_sufstats();
    Sigsq_prm()->add_observer(
        boost::bind(&ArStateModel::observe_residual_variance,
                    this));
  }

  //======================================================================
  ArStateModel::ArStateModel(const ArStateModel &rhs)
      : Model(rhs),
        StateModel(rhs),
        ArModel(rhs),
        state_transition_matrix_(new AutoRegressionTransitionMatrix(Phi_prm())),
        state_variance_matrix_(new UpperLeftCornerMatrix(
            Phi_prm()->size(), 1.0)),
        state_variance_is_current_(false),
        observation_matrix_(rhs.observation_matrix_),
        initial_state_mean_(rhs.initial_state_mean_),
        initial_state_variance_(rhs.initial_state_variance_),
        stationary_initial_distribution_(rhs.stationary_initial_distribution_)
  {
    DataPolicy::only_keep_sufstats();
    Sigsq_prm()->add_observer(
        boost::bind(&ArStateModel::observe_residual_variance,
                    this));
  }

  //======================================================================
  ArStateModel * ArStateModel::clone()const{
    return new ArStateModel(*this);}

  //======================================================================
  void ArStateModel::observe_state(const ConstVectorView then,
                                   const ConstVectorView now,
                                   int t){
    double y = now[0];
    const ConstVectorView &x(then);
    suf()->add_mixture_data(y, x, 1.0);
  }

  //======================================================================
  uint ArStateModel::state_dimension()const{
    return Phi_prm()->size();
  }

  //======================================================================
  void ArStateModel::simulate_state_error(VectorView eta, int t)const{
    eta = 0;
    eta[0] = rnorm() * sigma();
  }

  //======================================================================
  Ptr<SparseMatrixBlock> ArStateModel::state_transition_matrix(int t)const{
    return state_transition_matrix_;
  }

  //======================================================================
  Ptr<SparseMatrixBlock> ArStateModel::state_variance_matrix(int t)const{
    if(!state_variance_is_current_){
      state_variance_matrix_->set_value(sigsq());
    }
    return state_variance_matrix_;
  }

  //======================================================================
  SparseVector ArStateModel::observation_matrix(int t)const{
    return observation_matrix_;
  }

  //======================================================================
  Vector ArStateModel::initial_state_mean()const{
    if (initial_state_mean_.size() != state_dimension()) {
      report_error("mu_.size() != state_dimension() in "
                   "ArStateModel::initial_state_mean()");
    }
    return initial_state_mean_;
  }
  //======================================================================
  SpdMatrix ArStateModel::initial_state_variance()const{
    if (initial_state_variance_.nrow() != state_dimension()) {
      report_error("Sigma_.nrow() != state_dimension() in "
                   "ArStateModel::initial_state_mean()");
    }
    SpdMatrix & Sigma(const_cast<SpdMatrix &>(initial_state_variance_));

    if (stationary_initial_distribution_) {
      Vector gamma = autocovariance(state_dimension());
      Sigma.diag() = gamma[0];
      for(int i = 1; i < state_dimension(); ++i){
        Sigma.superdiag(i) = gamma[i];
      }
      Sigma.reflect();
    }
    return initial_state_variance_;
  }

  //======================================================================
  void ArStateModel::set_initial_state_mean(const Vector &mu){
    if(mu.size() != state_dimension()){
      report_error("attempt to set mu to the wrong size in "
                   "ArStateModel::set_initial_state_mean");
    }
    initial_state_mean_ = mu;
  }

  //======================================================================
  void ArStateModel::set_initial_state_variance(const SpdMatrix &Sigma){
    if(Sigma.nrow() != state_dimension()){
      report_error("attempt to set Sigma to the wrong size in "
                   "ArStateModel::set_initial_state_mean");
    }
    initial_state_variance_ = Sigma;
  }

}
