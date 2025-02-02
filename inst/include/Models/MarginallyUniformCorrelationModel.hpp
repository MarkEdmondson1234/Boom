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
#ifndef BOOM_MARGINALLY_UNIFORM_CORRELATION_MODEL_HPP_
#define BOOM_MARGINALLY_UNIFORM_CORRELATION_MODEL_HPP_

#include <Models/Policies/NullParamPolicy.hpp>
#include <Models/Policies/IID_DataPolicy.hpp>
#include <Models/Policies/PriorPolicy.hpp>
#include <Models/ModelTypes.hpp>
#include <Models/SpdParams.hpp>

namespace BOOM{

  class MarginallyUniformCorrelationModel
      : public NullParamPolicy,
        public IID_DataPolicy<SpdParams>,
        public PriorPolicy,
        public CorrelationModel
  {
   public:
    MarginallyUniformCorrelationModel(uint dim);
    MarginallyUniformCorrelationModel * clone() const override;

    virtual double pdf(Ptr<Data>, bool logscale)const;
    double logp(const Corr &)const override;
    uint dim()const;
    Corr sim()const;
   private:
    uint dim_;
  };
}
#endif // BOOM_MARGINALLY_UNIFORM_CORRELATION_MODEL_HPP_
