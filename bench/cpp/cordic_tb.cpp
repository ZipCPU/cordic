////////////////////////////////////////////////////////////////////////////////
//
// Filename:	cordic_tb.cpp
//
// Project:	A series of CORDIC related projects
//
// Purpose:	A quick test bench to determine if the basic cordic module
//		works.
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2017, Gisselquist Technology, LLC
//
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
//
#include <stdio.h>

#include <verilated.h>
#include <verilated_vcd_c.h>
#include "Vcordic.h"
#include "cordic.h"
#include "testb.h"

class	CORDIC_TB : public TESTB<Vcordic> {
	bool		m_debug;
public:

	CORDIC_TB(void) {
		m_debug = true;
		m_core->i_reset = 1;
		m_core->i_ce    = 1;
		m_core->i_xval  = (1ul<<(IW-1))-1;
		m_core->i_yval  = 0;
		m_core->i_phase = 0;
		m_core->i_aux   = 0;
		tick();
	}
};

const int	LGNSAMPLES=11;
const int	NSAMPLES=(1ul<<LGNSAMPLES);

int main(int  argc, char **argv) {
	Verilated::commandArgs(argc, argv);
	CORDIC_TB	*tb = new CORDIC_TB;
	int	pdata[NSAMPLES], xval[NSAMPLES], yval[NSAMPLES],
		ixval[NSAMPLES], iyval[NSAMPLES], idx, shift;
	double	scale;

	// This only works on DUT's with the aux flag turned on.
	assert(HAS_AUX);

	tb->opentrace("cordic_tb.vcd");
	tb->reset();

	shift = (8*sizeof(int)-OW);

	scale  = tb->m_core->i_xval * tb->m_core->i_xval;
	scale += tb->m_core->i_yval * tb->m_core->i_yval;
	scale  = sqrt(scale);

	idx = 0;
	for(int i=0; i<NSAMPLES; i++) {
		int	shift = (PW-(LGNSAMPLES-1));
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
		tb->tick();

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
	}

	tb->m_core->i_aux = 0;
	while(tb->m_core->o_aux) {
		shift = (8*sizeof(int)-OW);
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
		imag+=ixval[i] *ixval[i] +iyval[i] *iyval[i];
		// The magnitude we get on the output
		mag += xval[i] * xval[i] + yval[i] * yval[i];
		// The error between the value requested and the value resulting
		err = (dxval - xval[i]) * (dxval - xval[i]);
		err+= (dyval - yval[i]) * (dyval - yval[i]);

		// Let's run some other tests, to see if we managed to get the
		// gain right
		sumxy += dxval * xval[i];
		sumxy += dyval * yval[i];
		sumsq += xval[i] * xval[i] + yval[i]*yval[i];
		sumd  += dxval   *dxval    +dyval * dyval;
		averr += err;

		// printf("%6d %6d -> %f %f (predicted) -> %f err (%f)\n",
		//	xval[i], yval[i], dxval, dyval, err, averr);
		err = sqrt(err);
		if (err > mxerr)
			mxerr = err;
	}

	averr /= (NSAMPLES);
	averr  = sqrt(averr);
	mag   /= (NSAMPLES);
	mag    = sqrt(mag);
	imag  /= (NSAMPLES);
	imag   = sqrt(imag);

	bool	failed = false;

	// What average error do we expect?
	// int_-1/2^1/2 x^2 dx
	// = x^3/3 |_-1/2^1/2
	// = 1/24 + 1/24 = 1/12
	// Two of these added together is 2/12 per item
	printf("AVG Err: %.6f (%.6f Relative, %.4f expected)\n",
		averr, averr / mag, sqrt(QUANTIZATION_VARIANCE));
	if (averr > 1.5 * sqrt(QUANTIZATION_VARIANCE))
		failed = true;
	printf("MAX Err: %.6f (%.6f Relative)\n", mxerr, mxerr / mag);
	if (mxerr > 3.0)
		failed = true;
	printf("  Mag  : %.6f\n", mag);
	printf("(Gain) : %.6f\n", GAIN);
	printf("(alpha): %.6f\n", sumxy / sumsq);
	printf("SNR    : %.2f dB\n", 10.0*log(scale * scale / QUANTIZATION_VARIANCE)/log(10.0));
	if (fabs(sumxy / sumsq - 1.0) > 0.01)
		failed = true;

	if (failed) {
		printf("TEST FAILURE\n");
		exit(EXIT_FAILURE);
	} else {
		printf("SUCCESS!!\n");
		exit(EXIT_SUCCESS);
	}
}
