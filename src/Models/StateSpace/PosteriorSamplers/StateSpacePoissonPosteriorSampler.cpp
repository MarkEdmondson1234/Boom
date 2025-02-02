/*
  Copyright (C) 2005-2015 Steven L. Scott

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
#include <Models/StateSpace/PosteriorSamplers/StateSpacePoissonPosteriorSampler.hpp>
#include <Models/StateSpace/PosteriorSamplers/SufstatManager.hpp>
#include <distributions.hpp>

namespace BOOM {
  namespace StateSpace {

    class PoissonSufstatManager
      : public SufstatManagerBase {
     public:
      PoissonSufstatManager(StateSpacePoissonPosteriorSampler *sampler)
          : sampler_(sampler) {}

      void clear_complete_data_sufficient_statistics() override {
        sampler_->clear_complete_data_sufficient_statistics();
      }

      void update_complete_data_sufficient_statistics(int t) override {
        sampler_->update_complete_data_sufficient_statistics(t);
      }

     private:
      StateSpacePoissonPosteriorSampler *sampler_;
    };

  }  // namespace StateSpace

  namespace {
    typedef StateSpacePoissonPosteriorSampler SSPPS;
    typedef StateSpace::AugmentedPoissonRegressionData AugmentedData;
  }  // namespace

  SSPPS::StateSpacePoissonPosteriorSampler(
      StateSpacePoissonModel *model,
      Ptr<PoissonRegressionSpikeSlabSampler> observation_model_sampler,
      RNG &seeding_rng)
      : StateSpacePosteriorSampler(model, seeding_rng),
        model_(model),
        observation_model_sampler_(observation_model_sampler)
  {
    model_->register_data_observer(
        new StateSpace::PoissonSufstatManager(this));
    observation_model_sampler_->fix_latent_data(true);
  }

  void SSPPS::impute_nonstate_latent_data() {
    const std::vector<Ptr<AugmentedData> > &data(model_->dat());
    for (int t = 0; t < data.size(); ++t) {
      Ptr<AugmentedData> dp = data[t];
      double state_contribution =
          model_->observation_matrix(t).dot(model_->state(t));
      double regression_contribution =
          model_->observation_model()->predict(dp->x());

      double internal_neglog_final_event_time = 0;
      double internal_mixture_mean = 0;
      double internal_mixture_precision = 0;
      double neglog_final_interarrival_time = 0;
      double external_mixture_mean = 0;
      double external_mixture_precision = 0;
      data_imputer_.impute(
          rng(),
          dp->y(),
          dp->exposure(),
          state_contribution + regression_contribution,
          &internal_neglog_final_event_time,
          &internal_mixture_mean,
          &internal_mixture_precision,
          &neglog_final_interarrival_time,
          &external_mixture_mean,
          &external_mixture_precision);

      double total_precision = external_mixture_precision;
      double precision_weighted_sum =
          neglog_final_interarrival_time - external_mixture_mean;
      precision_weighted_sum *= external_mixture_precision;
      if (dp->y() > 0) {
        precision_weighted_sum +=
            (internal_neglog_final_event_time - internal_mixture_mean)
            * internal_mixture_precision;
        total_precision += internal_mixture_precision;
      }
      dp->set_latent_data(precision_weighted_sum / total_precision,
                          1.0 / total_precision);
      dp->set_offset(state_contribution);
    }
  }

  void SSPPS::clear_complete_data_sufficient_statistics() {
    observation_model_sampler_->clear_complete_data_sufficient_statistics();
  }

  void SSPPS::update_complete_data_sufficient_statistics(int t) {
    Ptr<AugmentedData> dp = model_->dat()[t];
    double precision_weighted_mean = dp->latent_data_value();
    precision_weighted_mean -= dp->offset();
    double precision = 1.0 / dp -> latent_data_variance();
    observation_model_sampler_->update_complete_data_sufficient_statistics(
        precision_weighted_mean * precision, precision, dp->x());
  }


}   // namespace BOOM
