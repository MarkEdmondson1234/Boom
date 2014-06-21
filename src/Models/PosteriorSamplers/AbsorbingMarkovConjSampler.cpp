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
#include <Models/PosteriorSamplers/AbsorbingMarkovConjSampler.hpp>
#include <distributions.hpp>
#include <cpputil/math_utils.hpp>
#include <cpputil/report_error.hpp>

namespace BOOM{
typedef AbsorbingMarkovConjSampler AMCS;
typedef MarkovConjSampler MCS;

AMCS::AbsorbingMarkovConjSampler(MarkovModel * Mod,
                               Ptr<ProductDirichletModel> Q,
                               Ptr<DirichletModel> pi0,
                               std::vector<uint> absorbing_states)
    :MCS(Mod, Q, pi0),
     mod_(Mod),
     abs_(absorbing_states, mod_->state_space_size()),
     trans_(abs_.complement())
{}

AMCS::AbsorbingMarkovConjSampler(MarkovModel * Mod,
                               Ptr<ProductDirichletModel> Q,
                                 std::vector<uint> absorbing_states)
    : MCS(Mod, Q),
      mod_(Mod),
      abs_(absorbing_states, mod_->state_space_size()),
      trans_(abs_.complement())

{
}

AMCS::AbsorbingMarkovConjSampler(MarkovModel * Mod,
                               const Mat & Nu,
                               std::vector<uint> absorbing_states)
    : MCS(Mod, Nu),
      mod_(Mod),
      abs_(absorbing_states, mod_->state_space_size()),
      trans_(abs_.complement())

{}

AMCS::AbsorbingMarkovConjSampler(MarkovModel * Mod,
                               const Mat & Nu,
                               const Vec & nu,
                               std::vector<uint> absorbing_states)
    : MCS(Mod, Nu, nu),
      mod_(Mod),
      abs_(absorbing_states, mod_->state_space_size()),
      trans_(abs_.complement())

{}

AMCS * AMCS::clone()const{return new AMCS(*this);}

double AMCS::logpri()const{
  uint S = mod_->state_space_size();
  Mat Q(mod_->Q());
  Vec nu(S);
  double ans=0;
  for(uint s=0; s<S; ++s){
    if(!abs_[s]){
      ans += ddirichlet(Q.row(s), Nu().row(s), true);
      if(ans == BOOM::negative_infinity()){
        ostringstream err;
        err << "Q(" << s << ") = " << Q.row(s) << endl
            << "Nu(" << s << ") = " << Nu().row(s) << endl
            << "ddirichlet(Q,Nu, true) = "
            << ddirichlet(Q.row(s), Nu().row(s), true)
            << endl;
        report_error(err.str());
      }
    }
  }

  if(mod_->pi0_fixed()) return ans;

  check_pi0();
  nu = this->nu();

  ans += ddirichlet(trans_.select(mod_->pi0()),
                    trans_.select(this->nu()),
                    true);
  return ans;
}


void AMCS::draw(){
  uint S = mod_->state_space_size();
  Mat Q(mod_->Q());
  Vec nu(S);
  for(uint s=0; s<S; ++s){
    if(!abs_[s]){
      nu = Nu().row(s)+ mod_->suf()->trans().row(s);
      Q.row(s) = rdirichlet_mt(rng(), nu);
    }else{
      Q.row(s) = 0.0;
      Q(s,s) = 1.0;
    }
  }

  mod_->set_Q(Q);
  if(mod_->pi0_fixed()) return;
  nu = this->nu() + mod_->suf()->init();
  nu = rdirichlet_mt(rng(), nu);
  mod_->set_pi0(nu);
}


void AMCS::find_posterior_mode(){

  uint S = mod_->state_space_size();
  Mat Q(mod_->Q());
  Vec nu(S);
  for(uint s=0; s<S; ++s){
    if(abs_[s]){
      Q.row(s) = 0.0;
      Q(s,s) = 1.0;
    }else{
      nu = Nu().row(s) + mod_->suf()->trans().row(s);
      Q.row(s) = mdirichlet(nu);
    }
  }
  mod_->set_Q(Q);

  if(mod_->pi0_fixed()) return;
  check_pi0();
  nu = this->nu() + mod_->suf()->init();
  mod_->set_pi0( mdirichlet(nu));

}
}
