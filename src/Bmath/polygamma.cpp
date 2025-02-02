/*
  Copyright (C) 2005 Steven L. Scott

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

/*
 *  Mathlib : A C Library of Special Functions
 *  Copyright (C) 1998 Ross Ihaka
 *  Copyright (C) 2000-2001 the R Development Core Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA.
 *
 *  SYNOPSIS
 *
 *    #include "Rnorm.h"
 *    void dpsifn(double x, int n, int kode, int m,
 *                double *ans, int *nz, int *ierr)
 *    double digamma(double x);
 *    double trigamma(double x)
 *    double tetragamma(double x)
 *    double pentagamma(double x)
 *
 *  DESCRIPTION
 *
 *    Compute the derivatives of the psi function
 *    and polygamma functions.
 *
 *    The following definitions are used in dpsifn:
 *
 *    Definition 1
 *
 *       psi(x) = d/dx (ln(gamma(x)),  the first derivative of
 *                                     the log gamma function.
 *
 *    Definition 2
 *                   k   k
 *       psi(k,x) = d /dx (psi(x)),    the k-th derivative
 *                                     of psi(x).
 *
 *
 *    "dpsifn" computes a sequence of scaled derivatives of
 *    the psi function; i.e. for fixed x and m it computes
 *    the m-member sequence
 *
 *                (-1)^(k+1) / gamma(k+1) * psi(k,x)
 *                   for k = n,...,n+m-1
 *
 *    where psi(k,x) is as defined above.   For kode=1, dpsifn
 *    returns the scaled derivatives as described.  kode=2 is
 *    operative only when k=0 and in that case dpsifn returns
 *    -psi(x) + ln(x).  That is, the logarithmic behavior for
 *    large x is removed when kode=2 and k=0.  When sums or
 *    differences of psi functions are computed the logarithmic
 *    terms can be combined analytically and computed separately
 *    to help retain significant digits.
 *
 *    Note that dpsifn(x, 0, 1, 1, ans) results in ans = -psi(x).
 *
 *  INPUT
 *
 *      x     - argument, x > 0.
 *
 *      n     - first member of the sequence, 0 <= n <= 100
 *              n == 0 gives ans(1) = -psi(x)       for kode=1
 *                                    -psi(x)+ln(x) for kode=2
 *
 *      kode  - selection parameter
 *              kode == 1 returns scaled derivatives of the
 *              psi function.
 *              kode == 2 returns scaled derivatives of the
 *              psi function except when n=0. In this case,
 *              ans(1) = -psi(x) + ln(x) is returned.
 *
 *      m     - number of members of the sequence, m >= 1
 *
 *  OUTPUT
 *
 *      ans   - a vector of length at least m whose first m
 *              components contain the sequence of derivatives
 *              scaled according to kode.
 *
 *      nz    - underflow flag
 *              nz == 0, a normal return
 *              nz != 0, underflow, last nz components of ans are
 *                       set to zero, ans(m-k+1)=0.0, k=1,...,nz
 *
 *      ierr  - error flag
 *              ierr=0, a normal return, computation completed
 *              ierr=1, input error,     no computation
 *              ierr=2, overflow,        x too small or n+m-1 too
 *                      large or both
 *              ierr=3, error,           n too large. dimensioned
 *                      array trmr(nmax) is not large enough for n
 *
 *    The nominal computational accuracy is the maximum of unit
 *    roundoff (d1mach(4)) and 1e-18 since critical constants
 *    are given to only 18 digits.
 *
 *    The basic method of evaluation is the asymptotic expansion
 *    for large x >= xmin followed by backward recursion on a two
 *    term recursion relation
 *
 *           w(x+1) + x^(-n-1) = w(x).
 *
 *    this is supplemented by a series
 *
 *           sum( (x+k)^(-n-1) , k=0,1,2,... )
 *
 *    which converges rapidly for large n. both xmin and the
 *    number of terms of the series are calculated from the unit
 *    roundoff of the machine environment.
 *
 *  AUTHOR
 *
 *    Amos, D. E.  (Fortran)
 *    Ross Ihaka   (C Translation)
 *
 *  REFERENCES
 *
 *    Handbook of Mathematical Functions,
 *    National Bureau of Standards Applied Mathematics Series 55,
 *    Edited by M. Abramowitz and I. A. Stegun, equations 6.3.5,
 *    6.3.18, 6.4.6, 6.4.9 and 6.4.10, pp.258-260, 1964.
 *
 *    D. E. Amos, (1983). "A Portable Fortran Subroutine for
 *    Derivatives of the Psi Function", Algorithm 610,
 *    TOMS 9(4), pp. 494-502.
 *
 *    Routines called: d1mach, i1mach.
 */

#include "nmath.hpp"
namespace Rmath{

static
void dpsifn(double x, int n, int kode, int m, double *ans, int *nz, int *ierr)
{
    const double bvalues[] = {  /* Bernoulli Numbers */
         1.00000000000000000e+00,
        -5.00000000000000000e-01,
         1.66666666666666667e-01,
        -3.33333333333333333e-02,
         2.38095238095238095e-02,
        -3.33333333333333333e-02,
         7.57575757575757576e-02,
        -2.53113553113553114e-01,
         1.16666666666666667e+00,
        -7.09215686274509804e+00,
         5.49711779448621554e+01,
        -5.29124242424242424e+02,
         6.19212318840579710e+03,
        -8.65802531135531136e+04,
         1.42551716666666667e+06,
        -2.72982310678160920e+07,
         6.01580873900642368e+08,
        -1.51163157670921569e+10,
         4.29614643061166667e+11,
        -1.37116552050883328e+13,
         4.88332318973593167e+14,
        -1.92965793419400681e+16
    };
    const double *b = (double *)&bvalues -1; /* ==> b[1] = bvalues[0], etc */
    const int nmax = 100;

    int i, j, k, mm, mx, nn, np, nx, fn;
    double arg, den, elim, eps, fln, fx, rln, rxsq,
        r1m4, r1m5, s, slope, t, ta, tk, tol, tols, tss, tst,
        tt, t1, t2, wdtol, xdmln, xdmy, xinc, xln, xm, xmin,
        xq, yint;
    double trm[23], trmr[101];

    *ierr = 0;
    if (x <= 0.0 || n < 0 || kode < 1 || kode > 2 || m < 1) {
        *ierr = 1;
        return;
    }

    /* fortran adjustment */
    ans--;

    *nz = 0;
    mm = m;
    nx = std::min(-i1mach(15), i1mach(16));
    r1m5 = d1mach(5);
    r1m4 = d1mach(4) * 0.5;
    wdtol = std::max(r1m4, 0.5e-18);

    /* elim = approximate exponential over and underflow limit */

    elim = 2.302 * (nx * r1m5 - 3.0);
    xln = log(x);
    for(;;) {
        nn = n + mm - 1;
        fn = nn;
        t = (fn + 1) * xln;

        /* overflow and underflow test for small and large x */

        if (fabs(t) > elim) {
            if (t <= 0.0) {
                *nz = 0;
                *ierr = 2;
                return;
            }
        }
        else {
            if (x < wdtol) {
                ans[1] = pow(x, -n-1.0);
                if (mm != 1) {
                    for(i = 2, k = 1; i <= mm ; i++, k++)
                        ans[k+1] = ans[k] / x;
                }
                if (n == 0 && kode == 2)
                    ans[1] += xln;
                return;
            }

            /* compute xmin and the number of terms of the series,  fln+1 */

            rln = r1m5 * i1mach(14);
            rln = std::min(rln, 18.06);
            fln = std::max(rln, 3.0) - 3.0;
            yint = 3.50 + 0.40 * fln;
            slope = 0.21 + fln * (0.0006038 * fln + 0.008677);
            xm = yint + slope * fn;
            mx = (int)xm + 1;
            xmin = mx;
            if (n != 0) {
                xm = -2.302 * rln - std::min(0.0, xln);
                arg = xm / n;
                arg = std::min(0.0, arg);
                eps = exp(arg);
                xm = 1.0 - eps;
                if (fabs(arg) < 1.0e-3)
                    xm = -arg;
                fln = x * xm / eps;
                xm = xmin - x;
                if (xm > 7.0 && fln < 15.0)
                    break;
            }
            xdmy = x;
            xdmln = xln;
            xinc = 0.0;
            if (x < xmin) {
                nx = (int)x;
                xinc = xmin - nx;
                xdmy = x + xinc;
                xdmln = log(xdmy);
            }

            /* generate w(n+mm-1, x) by the asymptotic expansion */

            t = fn * xdmln;
            t1 = xdmln + xdmln;
            t2 = t + xdmln;
            tk = std::max(fabs(t), std::max(fabs(t1), fabs(t2)));
            if (tk <= elim)
                goto L10;
        }
        nz++;
        ans[mm] = 0.0;
        mm--;
        if (mm == 0)
            return;
    }
    nn = (int)fln + 1;
    np = n + 1;
    t1 = (n + 1) * xln;
    t = exp(-t1);
    s = t;
    den = x;
    for(i=1 ; i<=nn ; i++) {
        den += 1.;
        trm[i] = pow(den, (double)-np);
        s += trm[i];
    }
    ans[1] = s;
    if (n == 0 && kode == 2)
        ans[1] = s + xln;

    if (mm != 1) { /* generate higher derivatives, j > n */

        tol = wdtol / 5.0;
        for(j = 2; j <= mm; j++) {
            t = t / x;
            s = t;
            tols = t * tol;
            den = x;
            for(i=1 ; i<=nn ; i++) {
                den += 1.;
                trm[i] /= den;
                s += trm[i];
                if (trm[i] < tols)
                    break;
            }
            ans[j] = s;
        }
    }
    return;

  L10:  tss = exp(-t);
    tt = 0.5 / xdmy;
    t1 = tt;
    tst = wdtol * tt;
    if (nn != 0)
        t1 = tt + 1.0 / fn;
    rxsq = 1.0 / (xdmy * xdmy);
    ta = 0.5 * rxsq;
    t = (fn + 1) * ta;
    s = t * b[3];
    if (fabs(s) >= tst) {
        tk = 2.0;
        for(k = 4; k <= 22; k++) {
            t = t * ((tk + fn + 1)/(tk + 1.0))*((tk + fn)/(tk + 2.0)) * rxsq;
            trm[k] = t * b[k];
            if (fabs(trm[k]) < tst)
                break;
            s += trm[k];
            tk += 2.;
        }
    }
    s = (s + t1) * tss;
    if (xinc != 0.0) {

        /* backward recur from xdmy to x */

        nx = (int)xinc;
        np = nn + 1;
        if (nx > nmax) {
            *nz = 0;
            *ierr = 3;
            return;
        }
        else {
            if (nn==0)
                goto L20;
            xm = xinc - 1.0;
            fx = x + xm;

            /* this loop should not be changed. fx is accurate when x is small */
            for(i = 1; i <= nx; i++) {
                trmr[i] = pow(fx, (double)-np);
                s += trmr[i];
                xm -= 1.;
                fx = x + xm;
            }
        }
    }
    ans[mm] = s;
    if (fn == 0)
        goto L30;

    /* generate lower derivatives,  j < n+mm-1 */

    for(j = 2; j <= mm; j++) {
        fn--;
        tss *= xdmy;
        t1 = tt;
        if (fn!=0)
            t1 = tt + 1.0 / fn;
        t = (fn + 1) * ta;
        s = t * b[3];
        if (fabs(s) >= tst) {
            tk = 4 + fn;
            for(k=4 ; k<=22 ; k++) {
                trm[k] = trm[k] * (fn + 1) / tk;
                if (fabs(trm[k]) < tst)
                    break;
                s += trm[k];
                tk += 2.;
            }
        }
        s = (s + t1) * tss;
        if (xinc != 0.0) {
            if (fn == 0)
                goto L20;
            xm = xinc - 1.0;
            fx = x + xm;
            for(i=1 ; i<=nx ; i++) {
                trmr[i] = trmr[i] * fx;
                s += trmr[i];
                xm -= 1.;
                fx = x + xm;
            }
        }
        mx = mm - j + 1;
        ans[mx] = s;
        if (fn == 0)
            goto L30;
    }
    return;

  L20:  
    for(i = 1; i <= nx; i++)
        s += 1. / (x + nx - i);

  L30:
    if (kode!=2)
        ans[1] = s - xdmln;
    else if (xdmy != x) {
        xq = xdmy / x;
        ans[1] = s - log(xq);
    }
    return;
}

double digamma(double x)
{
    double ans;
    int nz, ierr;
    if(ISNAN(x)) return x;
    dpsifn(x, 0, 1, 1, &ans, &nz, &ierr);
    if(ierr != 0) {
        errno = EDOM;
        return -numeric_limits<double>::max();
    }
    return -ans;
}

double trigamma(double x)
{
    double ans;
    int nz, ierr;
    if(ISNAN(x)) return x;
    dpsifn(x, 1, 1, 1, &ans, &nz, &ierr);
    if(ierr != 0) {
        errno = EDOM;
        return -numeric_limits<double>::max();
    }
    return ans;
}

double tetragamma(double x)
{
    double ans;
    int nz, ierr;
    if(ISNAN(x)) return x;
    dpsifn(x, 2, 1, 1, &ans, &nz, &ierr);
    if(ierr != 0) {
        errno = EDOM;
        return -numeric_limits<double>::max();
    }
    return -2.0 * ans;
}

double pentagamma(double x)
{
    double ans;
    int nz, ierr;
    if(ISNAN(x)) return x;
    dpsifn(x, 3, 1, 1, &ans, &nz, &ierr);
    if(ierr != 0) {
        errno = EDOM;
        return -numeric_limits<double>::max();
    }
    return 6.0 * ans;
}
}

