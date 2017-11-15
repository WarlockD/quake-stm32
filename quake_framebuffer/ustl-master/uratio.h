// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2016 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once
#include "sostream.h"

#if HAVE_CPP11	// ratio requires constexpr
namespace ustl {

template <intmax_t N, intmax_t D = 1>
struct ratio {
    static_assert (D, "denominator cannot be zero");
    using type = ratio<N,D>;
    enum : intmax_t { init_gcd = gcd(N,D) };
    static constexpr intmax_t num = N*sign(D) / init_gcd;
    static constexpr intmax_t den = absv(D) / init_gcd;
    void text_write (ostringstream& os) const
	{ os.format ("%jd/%jd", num, den); }
};

//{{{ Arithmetic operations --------------------------------------------

namespace {

template <typename R1, typename R2>
struct __ratio_add_impl {
    enum : intmax_t {
	den_gcd = gcd(R1::den, R2::den),
	n1 = R2::den/den_gcd * R1::num,
	n2 = R1::den/den_gcd * R2::num,
	nn = n1 + n2,
	nd = R1::den/den_gcd * R2::den
    };
};

template <typename R1, typename R2>
struct __ratio_multiply_impl {
    enum : intmax_t {
	gcd1 = gcd(R1::num, R2::den),
	gcd2 = gcd(R2::num, R1::den),
	nn = (R1::num/gcd1) * (R2::num/gcd2),
	nd = (R1::den/gcd2) * (R2::den/gcd1)
    };
};

} // namespace

template <typename R1, typename R2>
struct ratio_add : public ratio<
    __ratio_add_impl<R1,R2>::nn,
    __ratio_add_impl<R1,R2>::nd> {};

template <typename R1, typename R2>
struct ratio_multiply : public ratio<
    __ratio_multiply_impl<R1,R2>::nn,
    __ratio_multiply_impl<R1,R2>::nd> {};

template <typename R1, typename R2>
struct ratio_subtract : public ratio_add<R1, ratio<-R2::num,R2::den>> {};
template <typename R1, typename R2>
struct ratio_divide : public ratio_multiply<R1, ratio<R2::den,R2::num>> {};

//}}}-------------------------------------------------------------------
//{{{ Comparators

template <typename R1, typename R2>
struct ratio_equal : public integral_constant<bool,
    R1::num == R2::num && R1::den == R2::den> {};

template <typename R1, typename R2>
struct ratio_not_equal : public integral_constant<bool,
    !ratio_equal<R1,R2>::value> {};

template <typename R1, typename R2>
struct ratio_less : public integral_constant<bool,
    ratio_subtract<R1,R2>::num < 0> {};

template <typename R1, typename R2>
struct ratio_less_equal : public integral_constant<bool,
    ratio_subtract<R1,R2>::num <= 0> {};

template <typename R1, typename R2>
struct ratio_greater : public integral_constant<bool,
    !ratio_less_equal<R1,R2>::value> {};

template <typename R1, typename R2>
struct ratio_greater_equal : public integral_constant<bool,
    !ratio_less<R1,R2>::value> {};

//}}}-------------------------------------------------------------------
//{{{ Metric prefix ratios

using exa	= ratio<1000000000000000000, 1>;
using peta	= ratio<   1000000000000000, 1>;
using tera	= ratio<      1000000000000, 1>;
using giga	= ratio<         1000000000, 1>;
using mega	= ratio<            1000000, 1>;
using kilo	= ratio<               1000, 1>;
using hecto	= ratio<                100, 1>;
using deca	= ratio<                 10, 1>;
using deci	= ratio<1,                  10>;
using centi	= ratio<1,                 100>;
using milli	= ratio<1,                1000>;
using micro	= ratio<1,             1000000>;
using nano	= ratio<1,          1000000000>;
using pico	= ratio<1,       1000000000000>;
using femto	= ratio<1,    1000000000000000>;
using atto	= ratio<1, 1000000000000000000>;

//}}}-------------------------------------------------------------------
} // namespace ustl
#endif // HAVE_CPP11
