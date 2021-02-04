#pragma once

#include "ipp.h"
#include <boost/align/aligned_allocator.hpp>
#include <vector>

namespace Ipp_Filter {
	template <typename T>
	using aligned_vector = std::vector<T, boost::alignment::aligned_allocator<T, 64>>;
	typedef aligned_vector<Ipp64f> aligned_vec_64f;

	void check_status(IppStatus st);
	aligned_vec_64f createJaehne(int len, double magn);

	class FIR_StreamFilter
	{
	public:
		FIR_StreamFilter(const std::vector<double>& a, IppAlgType algType = ippAlgDirect);
		~FIR_StreamFilter();

		void reset();
		size_t taps_length() const;

		//! perform in-place filtering
		//! that is not working yet
		//void filter(aligned_vec_64f& v)
		//{
		//	// ippsFIRSR_32f(src + TAPS_LEN - 1, dst, LEN, pSpec, src, NULL, buf);
		//	check_status(ippsFIRSR_64f(v.data() + m_taps.size() - 1, v.data(), static_cast<int>(v.size() - m_taps.size() + 1), m_Spec, v.data(), nullptr, m_buffer.data()));
		//}

		void filter(const aligned_vec_64f& x, aligned_vec_64f& y);
		static std::vector<double> createLowPass(double frequency, int tapslen, IppWinType winType = ippWinBartlett);

	private:
		aligned_vector<Ipp64f> m_DlySrc;
		aligned_vector<Ipp64f> m_DlyDst;
		aligned_vector<Ipp64f> m_taps;
		aligned_vector<Ipp8u> m_buffer;
		IppsFIRSpec_64f* m_Spec = nullptr;
	};
};
