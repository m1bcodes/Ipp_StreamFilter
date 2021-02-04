#include <vector>
#include <string>

#include "ipp.h"

#include "FIR_StreamFilter.h"

Ipp_Filter::FIR_StreamFilter::FIR_StreamFilter(const std::vector<double>& a, IppAlgType algType)
{
	int bufSize = 0, specSize = 0;

	m_taps.assign(a.begin(), a.end());
	m_DlySrc.resize(m_taps.size(), 0);
	m_DlyDst.resize(m_taps.size(), 0);

	check_status(ippsFIRSRGetSize(static_cast<int>(m_taps.size()), ipp64f, &specSize, &bufSize));
	m_Spec = (IppsFIRSpec_64f*)ippsMalloc_8u(specSize);
	m_buffer.resize(bufSize);
	check_status(ippsFIRSRInit_64f(m_taps.data(), static_cast<int>(m_taps.size()), algType, m_Spec));
}

Ipp_Filter::FIR_StreamFilter::~FIR_StreamFilter()
{
	ippsFree(m_Spec);
}

void Ipp_Filter::FIR_StreamFilter::reset()
{
	m_DlySrc.assign(m_DlySrc.size(), 0);
	m_DlyDst.assign(m_DlyDst.size(), 0);
}

size_t Ipp_Filter::FIR_StreamFilter::taps_length() const 
{
	return m_taps.size();
}

void Ipp_Filter::FIR_StreamFilter::filter(const aligned_vec_64f& x, aligned_vec_64f& y)
{
	check_status(ippsFIRSR_64f(x.data(), y.data(), static_cast<int>(x.size()), m_Spec, m_DlySrc.data(), m_DlyDst.data(), m_buffer.data()));
	m_DlySrc.swap(m_DlyDst);
}

std::vector<double> Ipp_Filter::FIR_StreamFilter::createLowPass(double frequency, int tapslen, IppWinType winType)
{
	int bufSize = 0;
	Ipp8u* pBuffer = NULL;
	Ipp64f* taps = ippsMalloc_64f(tapslen * sizeof(Ipp64f));

	/*computes tapsLen coefficients for lowpass FIR filter*/
	check_status(ippsFIRGenGetBufferSize(tapslen, &bufSize));
	pBuffer = ippsMalloc_8u(bufSize);

	check_status(ippsFIRGenLowpass_64f(frequency, taps, tapslen, winType, ippTrue, pBuffer));

	std::vector<double> rtaps(taps, taps + tapslen);
	ippsFree(pBuffer);
	ippsFree(taps);
	return rtaps;
}

void Ipp_Filter::check_status(IppStatus st)
{
	if (st != ippStsNoErr)
		throw std::exception(("Ipp error " + std::to_string(st) + ": " + ippGetStatusString(st)).c_str()); /* Go to Exit if IPP function returned status different from ippStsNoErr */
}

Ipp_Filter::aligned_vec_64f Ipp_Filter::createJaehne(int len, double magn)
{
	aligned_vec_64f v(len);
	check_status(ippsVectorJaehne_64f(v.data(), len, magn)); /* create a Jaehne vector */
	return v;
}
