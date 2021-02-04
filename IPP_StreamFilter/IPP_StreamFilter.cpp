// Intel_Ipp_Test.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.
//

#include "gtest/gtest.h"

#include "ipp.h"
#include <stdio.h>
#include <exception>
#include <vector>
#include <string>
#include <chrono>
#include <iostream>

#include "FIR_StreamFilter.h"
#include "IIR_StreamFilter.h"

TEST(FIR_Filter, InPlaceStream)
{
	//std::vector<double> refOut, testOut;

	//const int LEN = 100;
	//const int TAPS_LEN = 8;
	//const int nLoops = 4;
	//{
	//	IppsFIRSpec_32f* pSpec;
	//	float* src, * dst, * taps;
	//	Ipp8u* buf;
	//	int             specSize, bufSize;

	//	//get sizes of the spec structure and the work buffer
	//	ippsFIRSRGetSize(TAPS_LEN, ipp32f, &specSize, &bufSize);

	//	src = ippsMalloc_32f(LEN + TAPS_LEN - 1);
	//	for (int i = 0; i < LEN + TAPS_LEN - 1; i++) src[i] = 1;
	//	dst = src;
	//	taps = ippsMalloc_32f(TAPS_LEN);
	//	for (int i = 0; i < TAPS_LEN; i++) taps[i] = 1;
	//	pSpec = (IppsFIRSpec_32f*)ippsMalloc_8u(specSize);
	//	buf = ippsMalloc_8u(bufSize);

	//	//initialize the spec structure
	//	ippsFIRSRInit_32f(taps, TAPS_LEN, ippAlgDirect, pSpec);
	//	std::vector<Ipp32f> out;
	//	//apply the FIR filter
	//	for (int i = 0; i < 3; i++) {
	//		for (int i = 0; i < LEN; i++) src[i] = 2;
	//		ippsFIRSR_32f(src + TAPS_LEN - 1, dst, LEN, pSpec, src, NULL, buf);
	//		out.insert(out.end(), dst, dst + LEN);
	//	}
	//}

	//{
	//	int len = 1000;
	//	Ipp64f magn = 4095;
	//	Ipp64f rFreq = 0.002;
	//	int tapslen = 27;
	//	int i = 0;

	//	Ipp_Filter::FIR_StreamFilter filter(Ipp_Filter::FIR_StreamFilter::createLowPass(rFreq, tapslen), {});

	//	Ipp_Filter::aligned_vec_64f pDst(len + filter.taps_length() - 1);
	//	Ipp_Filter::aligned_vec_64f FIRDst;
	//	Ipp64f phase = 0;

	//	for (int i = 0; i < 10; i++) {
	//		Ipp_Filter::check_status(ippsTriangle_64f(pDst.data(), len, magn, rFreq, 0, &phase));
	//		auto startClock = std::chrono::steady_clock::now();
	//		filter.filter(pDst);
	//		FIRDst.insert(FIRDst.end(), pDst.begin(), pDst.begin() + len);
	//	}
	//}
	//EXPECT_EQ(refOut.size(), testOut.size());
	//for (size_t i = 0; i < refOut.size(); i++) {
	//	EXPECT_NEAR(refOut[i], testOut[i], 1e-12) << "value " << i;
	//}

}

TEST(FIR_Filter, not_in_place_stream)
{
	// reference code from example
	const int nLoops = 4;
	const int len = 1000;
	const Ipp64f magn = 4095;
	const Ipp64f rFreq = 0.002;
	const int tapslen = 27;

	std::vector<double> refOut, testOut;
	{
		/* Next two defines are created to simplify code reading and understanding */
#define check_sts(st) if((st) != ippStsNoErr) throw std::exception();; /* Go to Exit if IPP function returned status different from ippStsNoErr */

/* Results of ippMalloc() are not validated because IPP functions perform bad arguments check and will return an appropriate status  */
		Ipp64f pDst[len];
		int numIters = len;
		IppsFIRSpec_64f* pSpec = NULL;
		IppStatus status = ippStsNoErr;
		int i = 0, bufSize = 0, specSize = 0;
		Ipp8u* pBuffer = NULL;
		IppAlgType algType = ippAlgDirect;
		Ipp64f* pDlySrc = NULL;
		Ipp64f* pDlyDst = NULL;

		Ipp64f* FIRDst = ippsMalloc_64f(numIters * sizeof(Ipp64f));
		Ipp64f* taps = ippsMalloc_64f(tapslen * sizeof(Ipp64f));

		pDlySrc = ippsMalloc_64f(tapslen * sizeof(Ipp64f));
		check_sts(status = ippsZero_64f(pDlySrc, tapslen));

		pDlyDst = ippsMalloc_64f(tapslen * sizeof(Ipp64f));
		check_sts(status = ippsZero_64f(pDlyDst, tapslen));

		/*computes tapsLen coefficients for lowpass FIR filter*/
		check_sts(status = ippsFIRGenGetBufferSize(tapslen, &bufSize));

		pBuffer = ippsMalloc_8u(bufSize);

		check_sts(status = ippsFIRGenLowpass_64f(0.5*rFreq, taps, tapslen, ippWinBartlett, ippTrue, pBuffer));

		check_sts(status = ippsFIRSRGetSize(tapslen, ipp64f, &specSize, &bufSize));
		pSpec = (IppsFIRSpec_64f*)ippsMalloc_8u(specSize);
		pBuffer = ippsMalloc_8u(bufSize);

		check_sts(status = ippsFIRSRInit_64f(taps, tapslen, algType, pSpec));

		auto startClock = std::chrono::steady_clock::now();

		/* filter an input vector */
		Ipp64f phase = 0;
		for (int i = 0; i < nLoops; i++) {
			check_sts(status = ippsTriangle_64f(pDst, len, magn, rFreq, 0, &phase));

			check_sts(status = ippsFIRSR_64f(pDst, FIRDst, numIters, pSpec, pDlySrc, pDlyDst, pBuffer));

			auto tmp = pDlyDst; pDlyDst = pDlySrc; pDlySrc = tmp;
			refOut.insert(refOut.end(), FIRDst, FIRDst + len);
		}
		std::chrono::duration<double> t = std::chrono::steady_clock::now() - startClock;
		//std::cout << "Duration C: " << t.count() << "\n";

		ippsFree(FIRDst);
		ippsFree(taps);
	}

	// test code
	{
		int i = 0;

		Ipp_Filter::FIR_StreamFilter filter(Ipp_Filter::FIR_StreamFilter::createLowPass(0.5*rFreq, tapslen), {});
		Ipp_Filter::aligned_vec_64f pDst(len);
		Ipp_Filter::aligned_vec_64f FIRDst, FIRDst2(len);
		Ipp64f phase = 0;

		for (int i = 0; i < nLoops; i++) {
			Ipp_Filter::check_status(ippsTriangle_64f(pDst.data(), len, magn, rFreq, 0, &phase));
			auto startClock = std::chrono::steady_clock::now();
			filter.filter(pDst, FIRDst2);
			testOut.insert(testOut.end(), FIRDst2.begin(), FIRDst2.end());
		}
	}

	EXPECT_EQ(refOut.size(), testOut.size());
	for (size_t i = 0; i < refOut.size(); i++) {
		EXPECT_NEAR(refOut[i], testOut[i], 1e-12) << "value " << i;
	}
}

TEST(IIR_Filter, not_in_place_stream)
{
	const int NUMITERS = 150;
	const int numLoops = 4;

	std::vector<double> refOut, testOut;
	{
		int n = 0, i = 0, specSize = 0, bufferSize = 0;
		IppsIIRState_64f* pIIRState = NULL;  /* Pointer to Intel(R) IPP state structure */
		IppsFIRSpec_64f* pFIRSpec = NULL;  /* Pointer to FIR Spec structure */
		Ipp64f* x = ippsMalloc_64f(NUMITERS + 10),
			* y = ippsMalloc_64f(NUMITERS),
			* z = ippsMalloc_64f(NUMITERS);
		const double taps[] = {
			0.0051f, 0.0180f, 0.0591f, 0.1245f, 0.1869f, 0.2127f, 0.1869f,
			0.1245f, 0.0591f, 0.0180f, 0.0051f, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		};
		Ipp8u* pBuf = NULL; /* Pointer to the work buffers */

		for (n = 0; n < NUMITERS; ++n) x[n] = sin(IPP_2PI * n / 150);

		Ipp_Filter::check_status(ippsIIRGetStateSize_64f(10, &bufferSize));

		pBuf = ippsMalloc_8u(bufferSize);
		Ipp_Filter::check_status(ippsIIRInit_64f(&pIIRState, taps, 10, NULL, pBuf));

		for (int i = 0; i < numLoops; i++) {
			Ipp_Filter::check_status(ippsIIR_64f(x, y, NUMITERS, pIIRState));
			refOut.insert(refOut.end(), y, y + NUMITERS);
		}

		ippsFree(pBuf); pBuf = NULL;

		Ipp_Filter::check_status(ippsFIRSRGetSize(11, ipp64f, &specSize, &bufferSize));
		pFIRSpec = (IppsFIRSpec_64f*)ippsMalloc_8u(specSize);
		pBuf = ippsMalloc_8u(bufferSize);

		Ipp_Filter::check_status(ippsFIRSRInit_64f(taps, 11, ippAlgDirect, pFIRSpec));

		Ipp_Filter::check_status(ippsFIRSR_64f(x, z, NUMITERS, pFIRSpec, NULL, NULL, pBuf));
		ippsFree(pBuf);
		ippsFree(z);
		ippsFree(y);
		ippsFree(x);
		ippsFree(pIIRState);
	}
	{
		std::vector<double> b{ 0.0051, 0.0180, 0.0591, 0.1245, 0.1869, 0.2127, 0.1869, 0.1245, 0.0591, 0.0180, 0.0051 },
			a{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		Ipp_Filter::aligned_vec_64f x(NUMITERS+10), y(NUMITERS);
		for (int i = 0; i < NUMITERS; ++i) x[i] = sin(IPP_2PI * i / 150);

		Ipp_Filter::IIR_StreamFilter filter(b, a);

		for (int i = 0; i < numLoops; i++) {
			filter.filter(x, y);
			testOut.insert(testOut.end(), y.begin(), y.end());
		}
	}
	EXPECT_EQ(refOut.size(), testOut.size());
	for (size_t i = 0; i < refOut.size(); i++) {
		EXPECT_NEAR(refOut[i], testOut[i], 1e-12) << "value " << i;
	}
}

int main(int argc, char* argv[])
{
	const IppLibraryVersion* lib;
	IppStatus status;
	Ipp64u mask, emask;

	/* Init IPP library */
	ippInit();
	/* Get IPP library version info */
	lib = ippGetLibVersion();
	printf("%s %s\n", lib->Name, lib->Version);

	/* Get CPU features and features enabled with selected library level */
	status = ippGetCpuFeatures(&mask, 0);
	if (ippStsNoErr == status) {
		emask = ippGetEnabledCpuFeatures();
		printf("Features supported by CPU\tby IPP\n");
		printf("-----------------------------------------\n");
		printf("  ippCPUID_MMX        = ");
		printf("%c\t%c\t", (mask & ippCPUID_MMX) ? 'Y' : 'N', (emask & ippCPUID_MMX) ? 'Y' : 'N');
		printf("Intel(R) Architecture MMX technology supported\n");
		printf("  ippCPUID_SSE        = ");
		printf("%c\t%c\t", (mask & ippCPUID_SSE) ? 'Y' : 'N', (emask & ippCPUID_SSE) ? 'Y' : 'N');
		printf("Intel(R) Streaming SIMD Extensions\n");
		printf("  ippCPUID_SSE2       = ");
		printf("%c\t%c\t", (mask & ippCPUID_SSE2) ? 'Y' : 'N', (emask & ippCPUID_SSE2) ? 'Y' : 'N');
		printf("Intel(R) Streaming SIMD Extensions 2\n");
		printf("  ippCPUID_SSE3       = ");
		printf("%c\t%c\t", (mask & ippCPUID_SSE3) ? 'Y' : 'N', (emask & ippCPUID_SSE3) ? 'Y' : 'N');
		printf("Intel(R) Streaming SIMD Extensions 3\n");
		printf("  ippCPUID_SSSE3      = ");
		printf("%c\t%c\t", (mask & ippCPUID_SSSE3) ? 'Y' : 'N', (emask & ippCPUID_SSSE3) ? 'Y' : 'N');
		printf("Intel(R) Supplemental Streaming SIMD Extensions 3\n");
		printf("  ippCPUID_MOVBE      = ");
		printf("%c\t%c\t", (mask & ippCPUID_MOVBE) ? 'Y' : 'N', (emask & ippCPUID_MOVBE) ? 'Y' : 'N');
		printf("The processor supports MOVBE instruction\n");
		printf("  ippCPUID_SSE41      = ");
		printf("%c\t%c\t", (mask & ippCPUID_SSE41) ? 'Y' : 'N', (emask & ippCPUID_SSE41) ? 'Y' : 'N');
		printf("Intel(R) Streaming SIMD Extensions 4.1\n");
		printf("  ippCPUID_SSE42      = ");
		printf("%c\t%c\t", (mask & ippCPUID_SSE42) ? 'Y' : 'N', (emask & ippCPUID_SSE42) ? 'Y' : 'N');
		printf("Intel(R) Streaming SIMD Extensions 4.2\n");
		printf("  ippCPUID_AVX        = ");
		printf("%c\t%c\t", (mask & ippCPUID_AVX) ? 'Y' : 'N', (emask & ippCPUID_AVX) ? 'Y' : 'N');
		printf("Intel(R) Advanced Vector Extensions instruction set\n");
		printf("  ippAVX_ENABLEDBYOS  = ");
		printf("%c\t%c\t", (mask & ippAVX_ENABLEDBYOS) ? 'Y' : 'N', (emask & ippAVX_ENABLEDBYOS) ? 'Y' : 'N');
		printf("The operating system supports Intel(R) AVX\n");
		printf("  ippCPUID_AES        = ");
		printf("%c\t%c\t", (mask & ippCPUID_AES) ? 'Y' : 'N', (emask & ippCPUID_AES) ? 'Y' : 'N');
		printf("Intel(R) AES instruction\n");
		printf("  ippCPUID_SHA        = ");
		printf("%c\t%c\t", (mask & ippCPUID_SHA) ? 'Y' : 'N', (emask & ippCPUID_SHA) ? 'Y' : 'N');
		printf("Intel(R) SHA new instructions\n");
		printf("  ippCPUID_CLMUL      = ");
		printf("%c\t%c\t", (mask & ippCPUID_CLMUL) ? 'Y' : 'N', (emask & ippCPUID_CLMUL) ? 'Y' : 'N');
		printf("PCLMULQDQ instruction\n");
		printf("  ippCPUID_RDRAND     = ");
		printf("%c\t%c\t", (mask & ippCPUID_RDRAND) ? 'Y' : 'N', (emask & ippCPUID_RDRAND) ? 'Y' : 'N');
		printf("Read Random Number instructions\n");
		printf("  ippCPUID_F16C       = ");
		printf("%c\t%c\t", (mask & ippCPUID_F16C) ? 'Y' : 'N', (emask & ippCPUID_F16C) ? 'Y' : 'N');
		printf("Float16 instructions\n");
		printf("  ippCPUID_AVX2       = ");
		printf("%c\t%c\t", (mask & ippCPUID_AVX2) ? 'Y' : 'N', (emask & ippCPUID_AVX2) ? 'Y' : 'N');
		printf("Intel(R) Advanced Vector Extensions 2 instruction set\n");
		printf("  ippCPUID_AVX512F    = ");
		printf("%c\t%c\t", (mask & ippCPUID_AVX512F) ? 'Y' : 'N', (emask & ippCPUID_AVX512F) ? 'Y' : 'N');
		printf("Intel(R) Advanced Vector Extensions 3.1 instruction set\n");
		printf("  ippCPUID_AVX512CD   = ");
		printf("%c\t%c\t", (mask & ippCPUID_AVX512CD) ? 'Y' : 'N', (emask & ippCPUID_AVX512CD) ? 'Y' : 'N');
		printf("Intel(R) Advanced Vector Extensions CD (Conflict Detection) instruction set\n");
		printf("  ippCPUID_AVX512ER   = ");
		printf("%c\t%c\t", (mask & ippCPUID_AVX512ER) ? 'Y' : 'N', (emask & ippCPUID_AVX512ER) ? 'Y' : 'N');
		printf("Intel(R) Advanced Vector Extensions ER instruction set\n");
		printf("  ippCPUID_ADCOX      = ");
		printf("%c\t%c\t", (mask & ippCPUID_ADCOX) ? 'Y' : 'N', (emask & ippCPUID_ADCOX) ? 'Y' : 'N');
		printf("ADCX and ADOX instructions\n");
		printf("  ippCPUID_RDSEED     = ");
		printf("%c\t%c\t", (mask & ippCPUID_RDSEED) ? 'Y' : 'N', (emask & ippCPUID_RDSEED) ? 'Y' : 'N');
		printf("The RDSEED instruction\n");
		printf("  ippCPUID_PREFETCHW  = ");
		printf("%c\t%c\t", (mask & ippCPUID_PREFETCHW) ? 'Y' : 'N', (emask & ippCPUID_PREFETCHW) ? 'Y' : 'N');
		printf("The PREFETCHW instruction\n");
		printf("  ippCPUID_KNC        = ");
		printf("%c\t%c\t", (mask & ippCPUID_KNC) ? 'Y' : 'N', (emask & ippCPUID_KNC) ? 'Y' : 'N');
		printf("Intel(R) Xeon Phi(TM) Coprocessor instruction set\n");
	}

	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
