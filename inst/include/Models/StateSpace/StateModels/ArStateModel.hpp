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

#ifndef BOOM_AR_STATE_MODEL_HPP_
#define BOOM_AR_STATE_MODEL_HPP_

#include <Models/StateSpace/StateModels/StateModel.hpp>
#include <Models/TimeSeries/ArModel.hpp>

namespace BOOM{
  // A state space model based on a stationary AR(p) process.  The
  // initial state is populated with a set of IID data with variance
  // equal to the stationary variance of the AR(p) process.  The model
  // is
  // alpha[t] = \sum_{i = 1}^p rho[i] * alpha[t-i] + epsilon[t-1]
  //
  // with epsilon[t-1] \sim N(0, sigma^2). (the t-1 subscript on
  // epsilon matches that of Durbin and Koopman (2001) and Harvey
  // (1989).)
  //
  // The parameters of this model are the vector of autoregression
  // coefficients rho[1]...rho[p], and the innovation variance
  // sigma^2.
  //
  // The state at time t is (alpha[t], alpha[t-1], ..., alpha[t-p+1]).
  // The observation matrix is Z[t] = (1, 0, 0, ..., 0)^T.
  // The transition matrix is
  //    [ rho_1 rho_2 ... rho_p ]
  //    [     1     0         0 ]
  //    [     0     1         0 ]
  //    [     ...               ]
  //    [     0            1  0 ]
  //
  // The shift portion of the stat transition is deterministic, so the
  // one-dimensional error is multiplied by
  // R_t = [1 0 0 0 0]^T
  class ArStateModel
      : public StateModel,
        public ArModel
  {
   public:
    ArStateModel(int number_of_lags = 1);
    ArStateModel(const ArStateModel &rhs);
    ArStateModel * clone() const override;

    void observe_state(const ConstVectorView previous_state,
                               const ConstVectorView current_state,
                               int t) override;

    uint state_dimension()const override;
    void simulate_state_error(VectorView eta, int t)const override;

    Ptr<SparseMatrixBlock> state_transition_matrix(int t)const override;
    Ptr<SparseMatrixBlock> state_variance_matrix(int t)const override;

    SparseVector observation_matrix(int t)const override;

    Vector initial_state_mean()const override;
    SpdMatrix initial_state_variance()const override;

    void set_initial_state_mean(const Vector &mu);
    void set_initial_state_variance(const SpdMatrix &Sigma);
    void use_stationary_distribution_as_initial_distribution();

   private:
    Ptr<AutoRegressionTransitionMatrix> state_transition_matrix_;
    Ptr<UpperLeftCornerMatrix> state_variance_matrix_;
    bool state_variance_is_current_;
    SparseVector observation_matrix_;

    Vector initial_state_mean_;
    SpdMatrix initial_state_variance_;
    bool stationary_initial_distribution_;

    void observe_residual_variance() {
      state_variance_is_current_ = false;
    }
  };


}
#endif //  BOOM_AR_STATE_MODEL_HPP_
