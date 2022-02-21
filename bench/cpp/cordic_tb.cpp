////////////////////////////////////////////////////////////////////////////////
//
// Filename:	cordic_tb.cpp
// {{{
// Project:	A series of CORDIC related projects
//
// Purpose:	A quick test bench to determine if the basic cordic module
//		works.
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
// }}}
// Copyright (C) 2017-2022, Gisselquist Technology, LLC
// {{{
// This program is free software (firmware): you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTIBILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program.  (It's in the $(ROOT)/doc directory.  Run make with no
// target there if the PDF file isn't present.)  If not, see
// <http://www.gnu.org/licenses/> for a copy.
//
// License:	GPL, v3, as defined and found on www.gnu.org,
//		http://www.gnu.org/licenses/gpl.html
//
//
////////////////////////////////////////////////////////////////////////////////
//
// }}}
#include <stdio.h>

#include <verilated.h>
#include <verilated_vcd_c.h>
#ifdef	CLOCKS_PER_OUTPUT
# include "Vseqcordic.h"
# include "seqcordic.h"
# define BASECLASS Vseqcordic
#else
# include "Vcordic.h"
# include "cordic.h"
# define BASECLASS Vcordic
#endif
#include "fft.h"
#include "testb.h"

class	CORDIC_TB : public TESTB<BASECLASS> {
	bool		m_debug;
public:
	// CORDIC_TB constructor
	// {{{
	CORDIC_TB(void) {
		m_debug = true;
#ifdef	CLOCKS_PER_OUTPUT
		m_core->i_stb   = 0;
#else	// CLOCKS_PER_OUTPUT
		m_core->i_ce    = 1;
#endif	// CLOCKS_PER_OUTPUT
		m_core->i_xval  = (1ul<<(IW-1))-1;
		m_core->i_yval  = 0;
		m_core->i_phase = 0;
		m_core->i_aux   = 0;
#ifdef	HAS_RESET_WIRE
#ifdef	ASYNC_RESET
		m_core->i_areset_n = 0;
#else
		m_core->i_reset = 1;
#endif
		tick();
#endif
	}
	// }}}
};

const int	LGNSAMPLES=PW;
const int	NSAMPLES=(1ul<<LGNSAMPLES);

int main(int  argc, char **argv) {
	Verilated::commandArgs(argc, argv);
	CORDIC_TB	*tb = new CORDIC_TB;
	int	*pdata, *xval, *yval,
		*ixval, *iyval, idx;
	double	scale;

	pdata = new int[NSAMPLES];
	xval  = new int[NSAMPLES];
	yval  = new int[NSAMPLES];
	ixval  = new int[NSAMPLES];
	iyval  = new int[NSAMPLES];

	// This only works on DUT's with the aux flag turned on.
	assert(HAS_AUX);

	// Open a trace
	// {{{
#ifdef	CLOCKS_PER_OUTPUT
	tb->opentrace("seqcordic_tb.vcd");
#else
	tb->opentrace("cordic_tb.vcd");
#endif
	// }}}

	// Reset the design
	// {{{
	tb->reset();
	// }}}

	// scale
	// {{{
	scale  = tb->m_core->i_xval * (double)tb->m_core->i_xval;
	scale += tb->m_core->i_yval * (double)tb->m_core->i_yval;
	scale  = sqrt(scale);
	// }}}

	// Simulation loop for NSAMPLES time steps
	// {{{
	idx = 0;
	for(int i=0; i<NSAMPLES; i++) {
		int	shift = (PW-LGNSAMPLES);
		if (shift < 0) {
			int	sv = i;
			if (i & (1ul<<(-shift)))
				// Odd value, round down
				sv += (1ul<<(-shift-1))-1;
			else
				sv += (1ul<<(-shift-1));
			tb->m_core->i_phase = sv >> (-shift);
		} else
			tb->m_core->i_phase = i << shift;
		pdata[i] = tb->m_core->i_phase;
		ixval[i] = tb->m_core->i_xval;
		iyval[i] = tb->m_core->i_yval;
		tb->m_core->i_aux   = 1;

		// Step the clock
		// {{{
#ifdef	CLOCKS_PER_OUTPUT
		// Step by CLOCKS_PER_OUTPUT clock ticks
		// {{{
		tb->m_core->i_stb = 1;
		for(int j=0; j<CLOCKS_PER_OUTPUT-1; j++) {
			tb->tick();
			tb->m_core->i_stb = 0;
			assert(!tb->m_core->o_done);
		}

		tb->tick();
		assert(tb->m_core->o_done);
		assert(tb->m_core->o_aux);
		// }}}
#else
		tb->tick();
#endif
		// }}}

		// Copy the results to an array for later analysis
		// {{{
		if (tb->m_core->o_aux) {
			shift = (8*sizeof(int)-OW);
			// Make our values signed..
			xval[idx] = tb->m_core->o_xval << (shift);
			yval[idx] = tb->m_core->o_yval << (shift);
			xval[idx] >>= shift;
			yval[idx] >>= shift;
			// printf("%08x<<%d: %08x %08x\n", (unsigned)pdata[i], shift, xval[idx], yval[idx]);
			idx++;
		}
		// }}}
	}
	// }}}

	// Flush any final data through the system
	// {{{
#ifndef	CLOCKS_PER_OUTPUT
	tb->m_core->i_aux = 0;
	while(tb->m_core->o_aux) {
		int	shift = (8*sizeof(int)-OW);
		tb->m_core->i_aux   = 0;
		tb->tick();

		if (tb->m_core->o_aux) {
			xval[idx] = tb->m_core->o_xval << (shift);
			yval[idx] = tb->m_core->o_yval << (shift);
			xval[idx] >>= shift;
			yval[idx] >>= shift;
			// printf("%08x %08x\n", xval[idx], yval[idx]);
			idx++;
			assert(idx <= NSAMPLES);
		}
	}
#endif
	// }}}

	if (false) { // Dump data for offline analysis
		// {{{
		FILE *fdbg = fopen("cordicdbg.dbl","w");
		for(int k=0; k<NSAMPLES; k++) {
			int	wv[5];
			wv[0] = pdata[k];
			wv[1] = ixval[k];
			wv[2] = iyval[k];
			wv[3] = xval[k];
			wv[4] = yval[k];
			fwrite(wv, sizeof(int), 5, fdbg);
		}
		fclose(fdbg);
	}
	// }}}

	// Determine if we were "close" enough: maximum error and average error
	// {{{
	double	mxerr = 0.0, averr = 0.0, mag = 0, imag=0, sumxy = 0.0,
		sumsq = 0.0, sumd = 0.0;
	for(int i=0; i<NSAMPLES; i++) {
		int	odata[5], shift;
		double	ph, dxval, dyval, err;

		odata[0] = pdata[i];
		odata[1] = ixval[i];
		odata[2] = iyval[i];
		odata[3] = xval[i];
		odata[4] = yval[i];

		ph = odata[0];
		ph = ph * M_PI * 2.0 / (double)(1u<<PW);
		dxval = cos(ph) * ixval[i] - sin(ph) * iyval[i];
		dyval = sin(ph) * ixval[i] + cos(ph) * iyval[i];

		dxval *= GAIN;
		dyval *= GAIN;

		shift = (IW+1-OW);
		if (IW +1 > OW) {
			dxval *= 1./(double)(1u<<shift);
			dyval *= 1./(double)(1u<<shift);
		} else if (OW > IW+1) {
			dxval *= 1./(double)(1u>>(-shift));
			dyval *= 1./(double)(1u>>(-shift));
		}


		// Solve min_a sum (d-a*v)^2
		//	min_a sum d^2 + a*d*v + a^2 v*v
		// 0 = d*v + 2*a*v*v
		// a = sumxw / 2 / sumsq
		//
		// Measure the magnitude of what we placed into the input
		imag+=ixval[i] *(double)ixval[i] +iyval[i] *(double)iyval[i];
		// The magnitude we get on the output
		mag += xval[i] * (double)xval[i] + yval[i] * (double)yval[i];
		// The error between the value requested and the value resulting
		err = (dxval - xval[i]) * (dxval - xval[i]);
		err+= (dyval - yval[i]) * (dyval - yval[i]);

		// Let's run some other tests, to see if we managed to get the
		// gain right
		sumxy += dxval * xval[i];
		sumxy += dyval * yval[i];
		sumsq += xval[i] * (double)xval[i] + yval[i]*(double)yval[i];
		sumd  += dxval   *dxval    +dyval * dyval;
		averr += err;

		if (PW<10) {
		printf("%6d %6d -> %9.2f %9.2f (predicted) -> %f err (%f), mag=%f\n",
			xval[i], yval[i], dxval, dyval, err, averr, mag);
		}
		err = sqrt(err);
		if (err > mxerr)
			mxerr = err;
	}
	// }}}

	bool	failed = false;
	double	expected_err;

	expected_err = QUANTIZATION_VARIANCE
			+ PHASE_VARIANCE_RAD*scale*scale*GAIN*GAIN;

	// Scale the error to a per-sample error
	// {{{
	// Error _magnitude_ should *never* be negative--a simple internal check
	averr /= (NSAMPLES);
	averr  = sqrt(averr);
	if (mag <= 0) {
		printf("ERR: Negative magnitude, %f\n", mag);
		goto test_failed;
	}
	mag   /= (NSAMPLES);
	mag    = sqrt(mag);
	if (imag <= 0) {
		printf("ERR: Negative i-magnitude, %f\n", imag);
		goto test_failed;
	}
	imag  /= (NSAMPLES);
	imag   = sqrt(imag);
	// }}}

	// What average error do we expect?  and did we pass?

	// Report on the results
	// {{{
	// int_-1/2^1/2 x^2 dx
	// = x^3/3 |_-1/2^1/2
	// = 1/24 + 1/24 = 1/12
	// Two of these added together is 2/12 per item
	printf("AVG Err: %.6f Units (%.6f Relative, %.4f Units expected)\n",
		averr, averr / mag, sqrt(expected_err));
	if (averr > 1.5 * sqrt(expected_err))
		failed = true;
	printf("MAX Err: %.6f Units (%.6f Relative, %.6f threshold)\n", mxerr,
		mxerr / mag, 5.2*sqrt(expected_err));
	if (mxerr > 5.2 * sqrt(expected_err)) {
		printf("ERR: Maximum error is out of bounds\n");
		failed = true;
	}
	printf("  Mag  : %.6f\n", mag);
	printf("(Gain) : %.6f\n", GAIN);
	printf("(alpha): %.6f\n", sumxy / sumsq);
	scale *= GAIN;
	printf("CNR    : %.2f dB (expected %.2f dB)\n",
		10.0*log(scale * scale
			/ (averr * averr))/log(10.0),
		BEST_POSSIBLE_CNR);
	if (fabs(sumxy / sumsq - 1.0) > 0.01) {
		printf("(alpha)is out of bounds!\n");
		goto test_failed;
	} if (failed)
		goto test_failed;
	// }}}

	// Estimate and check the spurious free dynamic range
	// {{{
	if ((PW < 26)&&(NSAMPLES == (1ul << PW))) {
		typedef	std::complex<double>	COMPLEX;
		COMPLEX	*outpt;
		const	unsigned long	FFTLEN=(1ul<<PW);

		outpt = new COMPLEX[FFTLEN];

		for(unsigned k=0; k<FFTLEN; k++) {
			outpt[k].real(xval[k]);
			outpt[k].imag(yval[k]);
		}

		// Now we need to do an FFT
		cfft((double *)outpt, FFTLEN);

		double	master, spur, tmp;

		// Master is the energy in the signal of interest
		master = norm(outpt[1]);

		// SPUR is the energy in any other FFT bin output
		spur = norm(outpt[0]);
		for(unsigned k=2; k<FFTLEN; k++) {
			tmp = norm(outpt[k]);
			if (tmp > spur)
				spur = tmp;
		}

		printf("SFDR = %7.2f dBc\n",
			10*log(master / spur)/log(10.));
	} else if (PW >= 26)
		printf("Too many phase bits ... skipping SFDR calculation\n");
	// }}}

	printf("SUCCESS!!\n");
	exit(EXIT_SUCCESS);

test_failed:
	printf("TEST FAILURE\n");
	exit(EXIT_FAILURE);
}

