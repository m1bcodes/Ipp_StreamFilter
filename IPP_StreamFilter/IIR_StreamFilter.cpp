#include "IIR_StreamFilter.h"

Ipp_Filter::IIR_StreamFilter::IIR_StreamFilter(const std::vector<double>& b, const std::vector<double>& a)
{
	int specSize = 0, bufferSize = 0;

	if (a.size() != b.size())
		throw std::exception("a and b filter coefficients must be equal size");
	int order = a.size()-1;
	for (double d : b) m_taps.push_back(d);
	for (double d : a) m_taps.push_back(d);

	Ipp_Filter::check_status(ippsIIRGetStateSize_64f(order, &bufferSize));
	m_buffer.resize(bufferSize);

	Ipp_Filter::check_status(ippsIIRInit_64f(&m_State, m_taps.data(), order, nullptr, m_buffer.data()));
}

Ipp_Filter::IIR_StreamFilter::~IIR_StreamFilter()
{
	// ippsFree(m_State);	// it crashes here, not sure what is going on
}

void Ipp_Filter::IIR_StreamFilter::filter(const aligned_vec_64f& x, aligned_vec_64f& y)
{
	Ipp_Filter::check_status(ippsIIR_64f(x.data(), y.data(), y.size(), m_State));
}
