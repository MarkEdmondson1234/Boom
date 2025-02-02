/*
  Copyright (C) 2005-2011 Steven L. Scott

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

#include <r_interface/boom_r_tools.hpp>
#include <r_interface/prior_specification.hpp>
#include <Models/BetaModel.hpp>
#include <Models/DiscreteUniformModel.hpp>
#include <Models/GammaModel.hpp>
#include <Models/GaussianModel.hpp>
#include <Models/MarkovModel.hpp>
#include <Models/PoissonModel.hpp>
#include <Models/PosteriorSamplers/MarkovConjSampler.hpp>
#include <Models/UniformModel.hpp>
#include <cpputil/math_utils.hpp>
#include <distributions.hpp>

namespace BOOM{
  namespace RInterface{
    SdPrior::SdPrior(SEXP prior)
        : prior_guess_(Rf_asReal(getListElement(prior, "prior.guess"))),
          prior_df_(Rf_asReal(getListElement(prior, "prior.df"))),
          initial_value_(Rf_asReal(getListElement(prior, "initial.value"))),
          fixed_(Rf_asLogical(getListElement(prior, "fixed"))),
          upper_limit_(Rf_asReal(getListElement(prior, "upper.limit")))
    {
      if (upper_limit_ < 0 || !R_FINITE(upper_limit_)) {
        upper_limit_ = BOOM::infinity();
      }
    }

    std::ostream & SdPrior::print(std::ostream &out)const{
      out << "prior_guess_   = " << prior_guess_ << std::endl
          << "prior_df_      = " << prior_df_ << std::endl
          << "initial_value_ = " << initial_value_ << std::endl
          << "fixed          = " << fixed_ << std::endl
          << "upper_limit_   = " << upper_limit_ << std::endl;
      return out;
    }

    NormalPrior::NormalPrior(SEXP prior)
        : mu_(Rf_asReal(getListElement(prior, "mu"))),
          sigma_(Rf_asReal(getListElement(prior, "sigma"))),
          initial_value_(Rf_asReal(getListElement(prior, "initial.value"))) {}

    std::ostream & NormalPrior::print(std::ostream &out)const{
      out << "mu =     " << mu_ << std::endl
          << "sigma_ = " << sigma_ << std::endl
          << "init   = " << initial_value_ << std::endl;
      return out;
    }

    BetaPrior::BetaPrior(SEXP prior)
        : a_(Rf_asReal(getListElement(prior, "a"))),
          b_(Rf_asReal(getListElement(prior, "b")))
    {}

    std::ostream & BetaPrior::print(std::ostream &out)const{
      out << "a = " << a_ << "b = " << b_;
      return out;
    }

    GammaPrior::GammaPrior(SEXP prior)
        : a_(Rf_asReal(getListElement(prior, "a"))),
          b_(Rf_asReal(getListElement(prior, "b")))
    {
      SEXP rinitial_value;
      PROTECT(rinitial_value = getListElement(prior, "initial.value"));
      if (rinitial_value == R_NilValue) {
        initial_value_ = Rf_asReal(rinitial_value);
      }else{
        initial_value_ = a_ / b_;
      }
      UNPROTECT(1);
    }

    std::ostream & GammaPrior::print(std::ostream &out)const{
      out << "a = " << a_ << "b = " << b_;
      return out;
    }


    MvnPrior::MvnPrior(SEXP prior)
        : mu_(ToBoomVector(getListElement(prior, "mean"))),
          Sigma_(ToBoomSpdMatrix(getListElement(prior, "variance")))
    {}

    std::ostream & MvnPrior::print(std::ostream &out)const{
      out << "mu: " << mu_ << std::endl
          << "Sigma:" << std::endl
          << Sigma_;
      return out;
    }

    Ar1CoefficientPrior::Ar1CoefficientPrior(SEXP prior)
        : NormalPrior(prior),
          force_stationary_(Rf_asLogical(getListElement(
              prior, "force.stationary"))),
          force_positive_(Rf_asLogical(getListElement(
              prior, "force.positive"))) {}

    std::ostream & Ar1CoefficientPrior::print(std::ostream &out)const{
      NormalPrior::print(out) << "force_stationary_ = "
                              << force_stationary_ << std::endl;
      return out;
    }

    ConditionalNormalPrior::ConditionalNormalPrior(SEXP prior)
        : mu_(Rf_asReal(getListElement(prior, "mu"))),
          sample_size_(Rf_asReal(getListElement(prior, "sample.size")))
    {}

    std::ostream & ConditionalNormalPrior::print(std::ostream & out)const{
      out << "prior mean: " << mu_ << std::endl
          << "prior sample size for prior mean:" << sample_size_;
      return out;
    }

    NormalInverseGammaPrior::NormalInverseGammaPrior(SEXP prior)
        : prior_mean_guess_(Rf_asReal(getListElement(
            prior, "mu.guess"))),
          prior_mean_sample_size_(Rf_asReal(getListElement(
              prior, "mu.guess.weight"))),
          sd_prior_(getListElement(prior, "sigma.prior"))
    {}

    std::ostream & NormalInverseGammaPrior::print(std::ostream &out)const{
      out << "prior_mean_guess        = " << prior_mean_guess_ << std::endl
          << "prior_mean_sample_size: = " << prior_mean_sample_size_
          << std::endl
          << "prior for sigma: " << std::endl
          << sd_prior_;
      return out;
    }

    DirichletPrior::DirichletPrior(SEXP prior)
        : prior_counts_(ToBoomVector(
            getListElement(prior, "prior.counts")))
    {}

    const Vector & DirichletPrior::prior_counts()const{
      return prior_counts_;
    }

    int DirichletPrior::dim()const{
      return prior_counts_.size();
    }

    MarkovPrior::MarkovPrior(SEXP prior)
        : transition_counts_(ToBoomMatrix(getListElement(
            prior, "prior.transition.counts"))),
          initial_state_counts_(ToBoomVector(getListElement(
              prior, "prior.initial.state.counts")))
    {}

    std::ostream & MarkovPrior::print(std::ostream &out)const{
      out << "prior transition counts: " << std::endl
          << transition_counts_ << std::endl
          << "prior initial state counts:" << std::endl
          << initial_state_counts_;
      return out;
    }

    MarkovModel * MarkovPrior::create_markov_model()const{
      MarkovModel * ans(new MarkovModel(transition_counts_.nrow()));
      Ptr<MarkovConjSampler> sampler(new MarkovConjSampler(
          ans, transition_counts_, initial_state_counts_));
      ans->set_method(sampler);
      return ans;
    }

    NormalInverseWishartPrior::NormalInverseWishartPrior(SEXP prior)
        : mu_guess_(ToBoomVector(getListElement(prior, "mean.guess"))),
          mu_guess_weight_(Rf_asReal(getListElement(
              prior, "mean.guess.weight"))),
          sigma_guess_(ToBoomSpdMatrix(getListElement(
              prior, "variance.guess"))),
          sigma_guess_weight_(Rf_asReal(getListElement(
              prior, "variance.guess.weight")))
    {}

    std::ostream & NormalInverseWishartPrior::print(std::ostream &out)const{
      out << "the prior mean for mu:" << std::endl
          << mu_guess_ << std::endl
          << "prior sample size for mu0: " << mu_guess_weight_ << std::endl
          << "prior sample size for Sigma_guess: " << sigma_guess_weight_
          << std::endl
          << "prior guess at Sigma: " << std::endl
          << sigma_guess_ << std::endl;
      return out;
    }

    MvnIndependentSigmaPrior::MvnIndependentSigmaPrior(SEXP prior)
        : mu_prior_(getListElement(prior, "mu.prior"))
    {
      int n = mu_prior_.mu().size();
      sigma_priors_.reserve(n);
      SEXP sigma_prior_list = getListElement(prior, "sigma.prior");
      for (int i = 0; i < n; ++i) {
        SdPrior sigma_prior(VECTOR_ELT(sigma_prior_list, i));
        sigma_priors_.push_back(sigma_prior);
      }
    }

    MvnDiagonalPrior::MvnDiagonalPrior(SEXP prior)
        : mean_(ToBoomVector(getListElement(prior, "mean"))),
          sd_(ToBoomVector(getListElement(prior, "sd")))
    {}

    DiscreteUniformPrior::DiscreteUniformPrior(SEXP prior)
        :lo_(Rf_asInteger(getListElement(prior, "lower.limit"))),
         hi_(Rf_asInteger(getListElement(prior, "upper.limit")))
    {
      if (hi_ < lo_) {
        report_error("hi < lo in DiscreteUniformPrior.");
      }
      log_normalizing_constant_ = -log(hi_ - lo_ + 1);
    }

    double DiscreteUniformPrior::logp(int value) const {
      if (value < lo_ || value > hi_) {
        return negative_infinity();
      }
      return log_normalizing_constant_;
    }

    PoissonPrior::PoissonPrior(SEXP prior)
        : lambda_(Rf_asReal(getListElement(prior, "mean"))),
          lo_(Rf_asReal(getListElement(prior, "lower.limit"))),
          hi_(Rf_asReal(getListElement(prior, "upper.limit")))
    {
      if (lambda_ <= 0) {
        report_error("lambda must be positive in PoissonPrior");
      }
      if (hi_ < lo_) {
        report_error("upper.limit < lower.limit in PoissonPrior.");
      }
      log_normalizing_constant_ = log(ppois(hi_, lambda_)
                                      - ppois(lo_ - 1, lambda_));

    }

    double PoissonPrior::logp(int value) const {
      return dpois(value, lambda_, true) - log_normalizing_constant_;
    }

    PointMassPrior::PointMassPrior(SEXP prior)
        : location_(Rf_asInteger(getListElement(prior, "location")))
    {}

    double PointMassPrior::logp(int value) const {
      return value == location_ ? 0 : negative_infinity();
    }

    Ptr<DoubleModel> create_double_model(SEXP r_spec) {
      if (Rf_inherits(r_spec, "GammaPrior")) {
        GammaPrior spec(r_spec);
        return new GammaModel(spec.a(), spec.b());
      }else if (Rf_inherits(r_spec, "BetaPrior")) {
        BetaPrior spec(r_spec);
        return new BetaModel(spec.a(), spec.b());
      }else if (Rf_inherits(r_spec, "NormalPrior")) {
        NormalPrior spec(r_spec);
        return new GaussianModel(spec.mu(), spec.sigma() * spec.sigma());
      }else if (Rf_inherits(r_spec, "UniformPrior")) {
        double lo = Rf_asReal(getListElement(r_spec, "lo"));
        double hi = Rf_asReal(getListElement(r_spec, "hi"));
        return new UniformModel(lo, hi);
      }
      report_error("Could not convert specification into a DoubleModel");
      return nullptr;
    }

    Ptr<DiffDoubleModel> create_diff_double_model(SEXP r_spec) {
      if (Rf_inherits(r_spec, "GammaPrior")) {
        GammaPrior spec(r_spec);
        return new GammaModel(spec.a(), spec.b());
      }else if (Rf_inherits(r_spec, "BetaPrior")) {
        BetaPrior spec(r_spec);
        return new BetaModel(spec.a(), spec.b());
      }else if (Rf_inherits(r_spec, "NormalPrior")) {
        NormalPrior spec(r_spec);
        return new GaussianModel(spec.mu(), spec.sigma() * spec.sigma());
      }else if (Rf_inherits(r_spec, "UniformPrior")) {
        double lo = Rf_asReal(getListElement(r_spec, "lo"));
        double hi = Rf_asReal(getListElement(r_spec, "hi"));
        return new UniformModel(lo, hi);
      }
      report_error("Could not convert specification into a DiffDoubleModel");
      return nullptr;
    }

    Ptr<IntModel> create_int_model(SEXP r_spec) {
      if (Rf_inherits(r_spec, "DiscreteUniformPrior")) {
        DiscreteUniformPrior spec(r_spec);
        return new DiscreteUniformModel(spec.lo(), spec.hi());
      } else if (Rf_inherits(r_spec, "PoissonPrior")) {
        PoissonPrior spec(r_spec);
        return new PoissonModel(spec.lambda());
      } else if (Rf_inherits(r_spec, "PointMassPrior")) {
        PointMassPrior spec(r_spec);
        return new DiscreteUniformModel(spec.location(), spec.location());
      } else {
        report_error("Could not convert specification into an IntModel.");
        return nullptr;
      }
    }

  } // namespace RInterface
} // namespace BOOM
