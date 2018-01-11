

#include <string>
#include <cstdint>
#include <cstdlib>
#include <cassert>

#include <windows.h>

namespace fftoa {


	template<typename T> struct diy_traits {};

	template<>
	struct diy_traits<float> {
		using floating_type = float;
		using significand_type = uint32_t;
		using exponent_type = int32_t;
		using mask_type = significand_type;
		using isize = ptrdiff_t;
		using usize = size_t;

		static constexpr usize diy_significand_size = 32;
		static constexpr usize significand_size = 23;
		static constexpr mask_type exponent_bias = 0x7F;
		static constexpr mask_type exponent_mask = 0x7F800000;
		static constexpr mask_type significand_mask = 0x007FFFFF;
		static constexpr mask_type hidden_bit = 0x00800000;
		static constexpr  isize min_power = (-36);
		// 10^-36, 10^-28, ..., 10^52
		static constexpr significand_type cached_powers_f[] = {
			0xaa242499, 0xfd87b5f3, 0xbce50865, 0x8cbccc09,
			0xd1b71759, 0x9c400000, 0xe8d4a510, 0xad78ebc6,
			0x813f3979, 0xc097ce7c, 0x8f7e32ce, 0xd5d238a5,
		};
		static constexpr int16_t cached_powers_e[] = {
			-151, -125, -98, -71, -45, -18, 8, 35, 62, 88, 115, 141,
		};



		union cvt_t {
			float v;
			uint32_t  u;
			constexpr cvt_t(float v) : v(v) { }
		};

		static constexpr uint32_t trasmute(floating_type f) { return cvt_t(f).u; }
	};	template<typename T>
	struct diy_fp {
		using traits = diy_traits<T>;
		using fty = typename traits::floating_type;
		using sigty = typename traits::significand_type;
		using expty = typename traits::exponent_type;
		using isize = typename traits::isize;
		using usize = typename traits::usize;
		static constexpr const char*DEC_DIGITS_LUT =
			"0001020304050607080910111213141516171819"
			"2021222324252627282930313233343536373839"
			"4041424344454647484950515253545556575859"
			"6061626364656667686970717273747576777879"
			"8081828384858687888990919293949596979899";
		expty e;
		sigty f;
		constexpr diy_fp(sigty f, expty e) : f(f), e(e) {}
		diy_fp(fty v) {
			auto u = traits::trasmute(v);

			expty biased_e = static_cast<expty>((u & traits::exponent_mask) >> traits::significand_size);
			sigty significand = u & traits::significand_mask;
			if (biased_e != 0) {
				f = significand + traits::hidden_bit;
				e = biased_e - traits::exponent_bias - traits::significand_size;
			}
			else {
				f = significand;
				e = 1 - traits::exponent_bias - traits::significand_size;
			}
		}
		// sub works on both
		inline constexpr diy_fp operator-(const diy_fp& rhs) const {
			return diy_fp(f - rhs.f, e);
		}
		constexpr inline diy_fp normalize() const {
			diy_fp res = *this;
			while ((res.f & (1 << (traits::diy_significand_size - 1))) == 0) {
				res.f <<= 1;
				res.e--;
			}
			return res;
		}
		constexpr inline diy_fp normalize_boundary() const {
			diy_fp res = *this;
			while ((res.f & (traits::hidden_bit << 1)) == 0) {
				res.f <<= 1;
				res.e--;
			}
			res.f <<= traits::diy_significand_size - traits::significand_size - 2;
			res.e -= traits::diy_significand_size - traits::significand_size - 2;
			return res;
		}
		constexpr inline std::pair<diy_fp, diy_fp> normalized_boundaries() const {
			diy_fp p1 = diy_fp((f << 1) + 1, e - 1).normalize_boundary();
			diy_fp m1 = (f == traits::hidden_bit) ? diy_fp((f << 2) - 1, e - 2) : diy_fp((f << 1) - 1, e - 1);
			m1.f <<= m1.e - p1.e;
			m1.e = p1.e;
			return std::make_pair(p1, m1);
		}
		static inline std::pair<diy_fp, isize> get_cached_power(expty e) {
			fty dk = static_cast<fty>(3 - traits::diy_significand_size - e) * static_cast<fty>(0.30102999566398114) - static_cast<fty>(traits::min_power + 1);
			isize k = static_cast<isize>(dk);
			if ((dk - k) > static_cast<fty>(0)) { k++; }// round
			usize index = (k >> 3) + 1;
			isize K = -(traits::min_power + static_cast<isize>(index << 3));
			diy_fp p(traits::cached_powers_f[index], static_cast<expty>(traits::cached_powers_e[index]));
			return std::make_pair(p, K);
		}
		static inline unsigned count_decimal_digit32(uint32_t n) {
			// Simple pure C++ implementation was faster than __builtin_clz version in this situation.
			if (n < 10) return 1;
			if (n < 100) return 2;
			if (n < 1000) return 3;
			if (n < 10000) return 4;
			if (n < 100000) return 5;
			if (n < 1000000) return 6;
			if (n < 10000000) return 7;
			if (n < 100000000) return 8;
			// Will not reach 10 digits in DigitGen()
			//if (n < 1000000000) return 9;
			//return 10;
			return 9;
		}
		static void grisu_round(char *buffer, isize len, sigty delta, sigty rest, sigty ten_kappa, sigty wp_w) {
			while (rest < wp_w && delta - rest >= ten_kappa &&
				(rest + ten_kappa < wp_w ||  /// closer
					wp_w - rest > rest + ten_kappa - wp_w)) {
				buffer[len - 1]--;
				rest += ten_kappa;
			}
		}
		static inline void digit_gen(const diy_fp& W, const diy_fp& Mp, sigty delta, char* buffer, isize* len, isize* K);
		static inline void grisu2(fty value, char* buffer, isize* length, isize* K) {
			const diy_fp v(value);
			auto nb = v.normalized_boundaries();
			const auto c_mk = get_cached_power(nb.first.e);
			*K = c_mk.second;
			const diy_fp W = v.normalize() * c_mk.first;
			diy_fp Wp = nb.first * c_mk.first;
			diy_fp Wm = nb.second * c_mk.first;
			Wm.f++;
			Wp.f--;
			digit_gen(W, Wp, Wp.f - Wm.f, buffer, length, K);
		}
		static inline char* write_exponent(isize K, char* buffer) {
			if (K < 0) {
				*buffer++ = '-';
				K = -K;
			}
			if (K >= 100) {
				*buffer++ = static_cast<char>('0' + static_cast<char>(K / 100));
				K %= 100;
				const char* d = DEC_DIGITS_LUT + K * 2;
				*buffer++ = d[0];
				*buffer++ = d[1];
			}
			else if (K >= 10) {
				const char* d = DEC_DIGITS_LUT + K * 2;
				*buffer++ = d[0];
				*buffer++ = d[1];
			}
			else
				*buffer++ = static_cast<char>('0' + static_cast<char>(K));
			return buffer;
		}		static inline char* prettify(char* buffer, isize length, isize k, isize maxDecimalPlaces) {
			const int kk = length + k;  // 10^(kk-1) <= v < 10^kk
			if (0 <= k && kk <= 21) {
				// 1234e7 -> 12340000000
				for (int i = length; i < kk; i++)
					buffer[i] = '0';
				buffer[kk] = '.';
				buffer[kk + 1] = '0';
				return &buffer[kk + 2];
			}
			else if (0 < kk && kk <= 21) {
				// 1234e-2 -> 12.34
				std::memmove(&buffer[kk + 1], &buffer[kk], static_cast<size_t>(length - kk));
				buffer[kk] = '.';
				if (0 > k + maxDecimalPlaces) {
					// When maxDecimalPlaces = 2, 1.2345 -> 1.23, 1.102 -> 1.1
					// Remove extra trailing zeros (at least one) after truncation.
					for (int i = kk + maxDecimalPlaces; i > kk + 1; i--)
						if (buffer[i] != '0')
							return &buffer[i + 1];
					return &buffer[kk + 2]; // Reserve one zero
				}
				else
					return &buffer[length + 1];
			}			else if (-6 < kk && kk <= 0) {
				// 1234e-6 -> 0.001234
				const int offset = 2 - kk;
				std::memmove(&buffer[offset], &buffer[0], static_cast<size_t>(length));
				buffer[0] = '0';
				buffer[1] = '.';
				for (int i = 2; i < offset; i++)
					buffer[i] = '0';
				if (length - kk > maxDecimalPlaces) {
					// When maxDecimalPlaces = 2, 0.123 -> 0.12, 0.102 -> 0.1
					// Remove extra trailing zeros (at least one) after truncation.
					for (int i = maxDecimalPlaces + 1; i > 2; i--)
						if (buffer[i] != '0')
							return &buffer[i + 1];
					return &buffer[3]; // Reserve one zero
				}
				else
					return &buffer[length + offset];
			}			else if (kk < -maxDecimalPlaces) {
				// Truncate to zero
				buffer[0] = '0';
				buffer[1] = '.';
				buffer[2] = '0';
				return &buffer[3];
			}
			else if (length == 1) {
				// 1e30
				buffer[1] = 'e';
				return write_exponent(kk - 1, &buffer[2]);
			}			else {
				// 1234e30 -> 1.234e33
				std::memmove(&buffer[2], &buffer[1], static_cast<size_t>(length - 1));
				buffer[1] = '.';
				buffer[length + 1] = 'e';
				return write_exponent(kk - 1, &buffer[0 + length + 2]);
			}		}		inline static char* to_string(fty value, char* buffer, int maxDecimalPlaces = 324) {
			assert(maxDecimalPlaces >= 1);
			fty d(value);
			if (d == 0.0f) {
				//	if (d.Sign())
				//		*buffer++ = '-';     // -0.0, Issue #289
				buffer[0] = '0';
				buffer[1] = '.';
				buffer[2] = '0';
				return &buffer[3];
			}
			else {
				if (value < 0) {
					*buffer++ = '-';
					value = -value;
				}
				int length, K;
				grisu2(value, buffer, &length, &K);
				return prettify(buffer, length, K, maxDecimalPlaces);
			}
		}	};
	static inline  diy_fp<float> operator*(const diy_fp<float>& lhs, const diy_fp<float>& rhs) {
		uint64_t tmp = static_cast<uint64_t>(lhs.f)* static_cast<uint64_t>(rhs.f);
		tmp += static_cast<uint64_t>(1) << 31; // round
		return diy_fp<float>(static_cast<uint32_t>(tmp >> 32), lhs.e + rhs.e + 32);
	}	template<typename T>	inline void diy_fp<T>::digit_gen(const diy_fp& W, const  diy_fp& Mp, sigty delta, char* buffer, isize* len, isize* K) {
		static constexpr const uint32_t POW10[] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };
		const  diy_fp one(uint64_t(1) << -Mp.e, Mp.e);
		const  diy_fp wp_w = Mp - W;
		uint32_t p1 = static_cast<uint32_t>(Mp.f >> -one.e);
		auto p2 = Mp.f & (one.f - 1);
		unsigned kappa = count_decimal_digit32(p1); // kappa in [0, 9]
		*len = 0;
		while (kappa > 0) {
			uint32_t d = 0;
			switch (kappa) {
			case  9: d = p1 / 100000000; p1 %= 100000000; break;
			case  8: d = p1 / 10000000; p1 %= 10000000; break;
			case  7: d = p1 / 1000000; p1 %= 1000000; break;
			case  6: d = p1 / 100000; p1 %= 100000; break;
			case  5: d = p1 / 10000; p1 %= 10000; break;
			case  4: d = p1 / 1000; p1 %= 1000; break;
			case  3: d = p1 / 100; p1 %= 100; break;
			case  2: d = p1 / 10; p1 %= 10; break;
			case  1: d = p1;              p1 = 0; break;
			default:;
			}
			if (d || *len)
				buffer[(*len)++] = static_cast<char>('0' + static_cast<char>(d));
			kappa--;
			auto tmp = (static_cast<sigty>(p1) << -one.e) + p2;
			if (tmp <= delta) {
				*K += kappa;
				grisu_round(buffer, *len, delta, tmp, POW10[kappa] << -one.e, wp_w.f);
				return;
			}
		}

		// kappa = 0

		for (;;) {
			p2 *= 10;
			delta *= 10;
			char d = static_cast<char>(p2 >> -one.e);
			if (d || *len)
				buffer[(*len)++] = static_cast<char>('0' + d);
			p2 &= one.f - 1;
			kappa--;
			if (p2 < delta) {
				*K += kappa;
				isize index = -static_cast<isize>(kappa);
				grisu_round(buffer, *len, delta, p2, one.f, wp_w.f * (index < 9 ? POW10[-static_cast<int>(kappa)] : 0));
				return;
			}
		}

	}}// 