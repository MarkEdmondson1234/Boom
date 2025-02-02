/*
  Copyright (C) 2007 Steven L. Scott

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

#ifndef BOOM_HMM_FILTER_HPP
#define BOOM_HMM_FILTER_HPP

#include <LinAlg/Types.hpp>
#include <LinAlg/Matrix.hpp>
#include <cpputil/Ptr.hpp>
#include <cpputil/RefCounted.hpp>
#include <Models/MarkovModel.hpp>
#include <distributions/rng.hpp>

namespace BOOM{
class Model;
class Data;
//class MarkovModel;
class EmMixtureComponent;
class HiddenMarkovModel;

class HmmFilter
    : private RefCounted{
 public:
  friend void intrusive_ptr_add_ref(HmmFilter *d){d->up_count();}
  friend void intrusive_ptr_release(HmmFilter *d){
      d->down_count(); if(d->ref_count()==0) delete d;}

  HmmFilter(std::vector<Ptr<MixtureComponent> >, Ptr<MarkovModel> );
  ~HmmFilter() override{}
  uint state_space_size()const;

  double initialize(const Data *);
  double loglike(const std::vector<Ptr<Data> > & );
  double fwd(const std::vector<Ptr<Data> > & );
  void bkwd_sampling(const std::vector<Ptr<Data> > &);
  void bkwd_sampling_mt(const std::vector<Ptr<Data> > &,
                        RNG & eng);
  virtual void allocate(Ptr<Data>, uint);
  virtual Vector state_probs(Ptr<Data>)const;
 protected:
  std::vector<Ptr<MixtureComponent> > models_;
  std::vector<Mat> P;
  Vector pi, logp, logpi, one;
  Matrix logQ;
  Ptr<MarkovModel> markov_;

};
//----------------------------------------------------------------------
class HmmSavePiFilter
    : public HmmFilter{
 public:
  HmmSavePiFilter(std::vector<Ptr<MixtureComponent> >,
                  Ptr<MarkovModel>,
                  std::map<Ptr<Data>, Vector> &pi_hist);
  void allocate(Ptr<Data> dp, uint h) override;
  Vector state_probs(Ptr<Data>)const override;
 private:
  std::map<Ptr<Data>, Vector> &pi_hist_;
};
//----------------------------------------------------------------------
class HmmEmFilter
    : public HmmFilter{
 public:
  HmmEmFilter(std::vector<Ptr<EmMixtureComponent> > , Ptr<MarkovModel>);
  virtual void bkwd_smoothing(const std::vector<Ptr<Data> > &);
 private:
  std::vector<Ptr<EmMixtureComponent> > models_;
};

}
#endif// BOOM_HMM_FILTER_HPP
