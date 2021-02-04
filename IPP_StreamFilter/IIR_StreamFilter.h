#pragma once

#include "FIR_StreamFilter.h"

namespace Ipp_Filter {
	class IIR_StreamFilter
	{
	public:
		IIR_StreamFilter(const std::vector<double>& b, const std::vector<double>& a);
		~IIR_StreamFilter();

		void filter(const aligned_vec_64f& x, aligned_vec_64f& y);
	private:
		aligned_vector<Ipp8u> m_buffer;
		aligned_vec_64f m_taps;
		IppsIIRState_64f* m_State = nullptr;
	};
}