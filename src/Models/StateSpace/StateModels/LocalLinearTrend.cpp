/*
  Copyright (C) 2005-2010 Steven L. Scott

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
#include <Models/StateSpace/StateModels/LocalLinearTrend.hpp>
#include <distributions.hpp>

namespace BOOM{
  namespace{
    typedef LocalLinearTrendStateModel LLTSM;
  }

  LLTSM::LocalLinearTrendStateModel()
      : ZeroMeanMvnModel(2),
        observation_matrix_(2),
        state_transition_matrix_(new LocalLinearTrendMatrix),
        state_variance_matrix_(new DenseSpd(ZeroMeanMvnModel::Sigma())),
        initial_state_mean_(2, 0.0),
        initial_state_variance_(2)
  {
    observation_matrix_[0] = 1;
  }

  LLTSM::LocalLinearTrendStateModel(const LLTSM &rhs)
      : ZeroMeanMvnModel(rhs),
        StateModel(rhs),
        observation_matrix_(rhs.observation_matrix_),
        state_transition_matrix_(rhs.state_transition_matrix_),
        state_variance_matrix_(rhs.state_variance_matrix_->clone()),
        initial_state_mean_(rhs.initial_state_mean_),
        initial_state_variance_(rhs.initial_state_variance_)
  {}

  LLTSM * LLTSM::clone()const{return new LLTSM(*this);}

  void LLTSM::observe_state(const ConstVectorView then,
                          const ConstVectorView now,
                          int time_now){
    check_dim(then);
    check_dim(now);

    Vector mu(2);
    state_transition_matrix_->multiply(VectorView(mu), then);
    Vector err = now - mu;

    suf()->update_raw(err);
  }

  void LLTSM::check_dim(const ConstVectorView &v)const{
    if(v.size() != 2){
      ostringstream err;
      err << "improper dimesion of ConstVectorView v = :"
          << v << endl
          << "in LocalLinearTrendStateModel.  Should be of dimension 2"
          << endl;
      report_error(err.str());
    }
  }

  void LLTSM::simulate_state_error(VectorView eta, int t)const{
    eta = ZeroMeanMvnModel::sim();
  }

  Ptr<SparseMatrixBlock> LLTSM::state_transition_matrix(int t)const{
    return state_transition_matrix_;}

  Ptr<SparseMatrixBlock> LLTSM::state_variance_matrix(int t)const{
    return state_variance_matrix_;}

  SparseVector LLTSM::observation_matrix(int)const{
    return observation_matrix_; }

  Vector LLTSM::initial_state_mean()const{return initial_state_mean_;}
  SpdMatrix LLTSM::initial_state_variance()const{return initial_state_variance_;}
  void LLTSM::set_initial_state_mean(const Vector &v){
    initial_state_mean_ = v; }
  void LLTSM::set_initial_state_variance(const SpdMatrix &Sigma){
    initial_state_variance_ = Sigma; }

  void LLTSM::set_Sigma(const SpdMatrix &Sigma){
    ZeroMeanMvnModel::set_Sigma(Sigma);
    state_variance_matrix_->set_matrix(Sigma);
  }

}
