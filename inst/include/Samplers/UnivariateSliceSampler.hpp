/*
  Copyright (C) 2006 Steven L. Scott

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

#include <Samplers/Sampler.hpp>
#include <Samplers/ScalarSliceSampler.hpp>
#include <LinAlg/Vector.hpp>
#include <functional>
#include <TargetFun/TargetFun.hpp>

namespace BOOM{

   // A "Univariate" slice sampler draws a vector one component at a
   // time.  If you just want to draw a scalar quantity then you want
   // a ScalarSliceSampler instead.
  class UnivariateSliceSampler : public Sampler {
   public:
    typedef std::function<double(const Vector &x)> Target;
    // Args:
    //   logdensity: The log of the un-normalized density function to
    //     be sampled.
    //   dim:  The dimension of the density to be sampled.
    //   suggested_dx: The initial suggested step size to use for each
    //     scalar slice sampler.
    //   unimodal: If 'true' the density is known to be unimodal.  If
    //     'false' then the density is potentially multi-modal.
    //   rng: A pointer to the random number generator that supplies
    //     randomness to this sampler.
    UnivariateSliceSampler(const Target &logdensity,
                           int dim,
                           double suggested_dx = 1.0,
                           bool unimodal = false,
                           RNG *rng = nullptr);
    Vector draw(const Vector &x) override;

   private:
    Target f_;
    std::vector<ScalarTargetFunAdapter> scalar_targets_;
    std::vector<ScalarSliceSampler> scalar_samplers_;
    Vector theta_;
  };

}
