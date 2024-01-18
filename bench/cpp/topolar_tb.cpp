////////////////////////////////////////////////////////////////////////////////
//
// Filename:	topolar_tb.cpp
// {{{
// Project:	A series of CORDIC related projects
//
// Purpose:	A quick test bench to determine if the rectangular to polar
//		cordic module works.
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
// }}}
// Copyright (C) 2017-2024, Gisselquist Technology, LLC
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
// }}}
// License:	GPL, v3, as defined and found on www.gnu.org,
// {{{
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
# include "Vseqpolar.h"
# include "seqpolar.h"
# define BASECLASS Vseqpolar
#else
# include "Vtopolar.h"
# include "topolar.h"
# define BASECLASS Vtopolar
#endif
#include "testb.h"

// TOPOLAR_TB
// {{{
class	TOPOLAR_TB : public TESTB<BASECLASS> {
	bool		m_debug;
public:

	TOPOLAR_TB(void) {
		// {{{
		m_debug = true;
#ifdef	WITH_RESET
#ifdef	ASYNC_RESET
		m_core->i_areset_n = 0;
#else
		m_core->i_reset = 1;
#endif
#endif
#ifdef	CLOCKS_PER_OUTPUT
		m_core->i_stb   = 0;
#else
		m_core->i_ce    = 1;
#endif
		m_core->i_xval  = 0;
		m_core->i_yval  = 0;
		m_core->i_aux   = 0;
		tick();
		// }}}
	}
};
// }}}

const int	LGNSAMPLES=PW;
const int	NSAMPLES=(1<<LGNSAMPLES);

int main(int  argc, char **argv) {
	// Declare necessary variables
	// {{{
	Verilated::commandArgs(argc, argv);
	TOPOLAR_TB	*tb = new TOPOLAR_TB;
	int	*ipdata, *ixval,  *iyval,
		*imag,   *ophase, *omag,
		idx, shift, pshift;
	double	*dpdata, sum_perr = 0.0;

	const	double	MAXPHASE = pow(2.0,PW);
	const	double	RAD_TO_PHASE = MAXPHASE / M_PI / 2.0;

	ipdata = new int[NSAMPLES];
	ixval  = new int[NSAMPLES];
	iyval  = new int[NSAMPLES];
	imag   = new int[NSAMPLES];
	omag   = new int[NSAMPLES];
	ophase = new int[NSAMPLES];
	dpdata = new double[NSAMPLES];
	// }}}

	// Open a trace
	// {{{

#ifdef	CLOCKS_PER_OUTPUT
	tb->opentrace("seqpolar_tb.vcd");
#else
	tb->opentrace("topolar_tb.vcd");
#endif
	// }}}

	tb->reset();

	// Run the simulation
	// {{{
	shift  = (8*sizeof(long)-OW);
	pshift = (8*sizeof(long)-PW);
	idx = 0;
	for(int i=0; i<NSAMPLES; i++) {
		// Feed the core with a number of test samples
		// {{{
		double	ph, cs, sn, mg;
		long	lv;

		lv = (((long)i) << (PW-(LGNSAMPLES-1)));
		ipdata[i] = (int)lv;
		ph = ipdata[i] * M_PI / (1ul << (PW-1));
		mg = ((1l<<(IW-1))-1);
		cs = mg * cos(ph);
		sn = mg * sin(ph);

		ixval[i] = (int)cs;
		iyval[i] = (int)sn;
		imag[i]  = (int)mg;
		dpdata[i] = atan2(iyval[i], ixval[i]);
		// dpdata[i] = ph;
		tb->m_core->i_xval  = ixval[i];
		tb->m_core->i_yval  = iyval[i];
		tb->m_core->i_aux   = 1;

#ifdef	CLOCKS_PER_OUTPUT
		// Wait for the result to be ready
		// {{{
		tb->m_core->i_stb = 1;
		for(int j=0; j<CLOCKS_PER_OUTPUT-1; j++) {
			tb->tick();
			tb->m_core->i_stb = 0;
			assert(!tb->m_core->o_done);
			assert( tb->m_core->o_busy);
		}

		tb->tick();
		assert(!tb->m_core->o_busy);
		assert(tb->m_core->o_done);
		assert(tb->m_core->o_aux);
		// }}}
#else
		// One data input per clock
		tb->tick();
#endif

		if (tb->m_core->o_aux) {
			// {{{
			long	lv;
			lv = (long)tb->m_core->o_mag;
			lv <<= shift;
			lv >>= shift;
			omag[idx]   = (int)lv;

			lv = tb->m_core->o_phase;
			lv <<= pshift;
			lv >>= pshift;
			ophase[idx] = (int)lv;
			//printf("%08x %08x -> %08x %08x\n",
			//	ixval[idx], iyval[idx],
			//	omag[idx], ophase[idx]);
			idx++;
			// }}}
		}
		// }}}
	}

#ifndef	CLOCKS_PER_OUTPUT
	// {{{
	tb->m_core->i_aux = 0;
	while(tb->m_core->o_aux) {
		tb->m_core->i_aux   = 0;
		tb->tick();

		if (tb->m_core->o_aux) {
			long	lv;
			lv = (long)tb->m_core->o_mag;
			lv <<= shift;
			lv >>= shift;
			omag[idx]   = (int)lv;

			lv = tb->m_core->o_phase;
			lv <<= pshift;
			lv >>= pshift;
			ophase[idx] = (int)lv;
			//printf("%08x %08x -> %08x %08x\n",
			//	ixval[idx], iyval[idx],
			//	omag[idx], ophase[idx]);
			idx++;
		}
	}
	// }}}
#endif
	// }}}

	// Get some statistics on the results
	// {{{
	double	mxperr = 0.0, mxverr = 0.0;
	for(int i=0; i<NSAMPLES; i++) {
		double	mgerr, epdata, dperr, emag;

		epdata = dpdata[i] * RAD_TO_PHASE;
		if (epdata < 0.0)
			epdata += MAXPHASE;
		dperr = ophase[i] - epdata;
		while (dperr > MAXPHASE/2.)
			dperr -= MAXPHASE;
		while (dperr < -MAXPHASE/2.)
			dperr += MAXPHASE;
		if (fabs(dperr) > mxperr)
			mxperr = fabs(dperr);
		sum_perr += dperr * dperr;

		emag = imag[i] * GAIN;// * sqrt(2);
		// if (IW+1 > OW)
		//	emag = emag / pow(2.,(IW-1-OW));
		// else if (OW > IW+1)
		//	emag = emag * pow(2.,(IW-1-OW));
		emag = imag[i] * pow(2.,(IW-1-OW));

		// omag should equal imag * GAIN
		mgerr = fabs(omag[i] - emag * GAIN);
		if (mgerr > mxverr)
			mxverr = mgerr;

		if (epdata > MAXPHASE/2)
			epdata -= MAXPHASE;
		//printf("%08x %08x -> %6d %08x/%12d [%9.6f %12.1f],[%9.6f %13.1f]\n",
		//	ixval[i], iyval[i],
		//	omag[i], ophase[i],ophase[i],
		//	emag, epdata,
		//	mgerr, dperr);
	}
	// }}}

	if(false) {
		// Generate an Octave-readable file for debugging (if necessary)
		// {{{
		FILE	*dbgfp;
		dbgfp = fopen("topolar.32t", "w");
		assert(dbgfp);

		for(int k=0; k<NSAMPLES; k++) {
			int	ovals[5];
			double	epdata, dperr;
			ovals[0] = ixval[k];
			ovals[1] = iyval[k];
			ovals[2] = omag[k];
			ovals[3] = ophase[k];

			epdata = dpdata[k] * RAD_TO_PHASE;
			dperr = ophase[k] - epdata;
			while (dperr > MAXPHASE/2.)
				dperr -= MAXPHASE;
			while (dperr < -MAXPHASE/2.)
				dperr += MAXPHASE;
			ovals[4] = (int)dperr;
			fwrite(ovals, sizeof(ovals[0]), sizeof(ovals)/sizeof(ovals[0]), dbgfp);
		}

		fclose(dbgfp);
		// }}}
	}

	sum_perr /= NSAMPLES;

	bool	failed_test = false;
	double	expected_phase_err;

	// First phase error: based upon the smallest arctan difference
	// between samples.
	//
	// expected_phase_err = atan2(1,(1<<(IW-1))) * RAD_TO_PHASE;
	// expected_phase_err *= expected_phase_err;
	//
	// expected_phase_err=pow((1<<(IW-1)),-2)*RAD_TO_PHASE*RAD_TO_PHASE/12.;
	//
	// expected_phase_err += QUANTIZATION_VARIANCE;
	// expected_phase_err *= (1./12.);
	expected_phase_err = 0.0;
	// Plus any truncation error in the in the phase values
	// 	Swap the units from radians to integer phase units
	expected_phase_err += PHASE_VARIANCE_RAD
				* RAD_TO_PHASE * RAD_TO_PHASE;
	expected_phase_err = sqrt(expected_phase_err);
	if (expected_phase_err < 1.0)
		expected_phase_err = 1.0;
	if (mxperr > 3.4 * expected_phase_err)
		failed_test = true;

	if (mxverr > 2.0 * sqrt(QUANTIZATION_VARIANCE))
		failed_test = true;

	printf("Max phase     error: %.2f (%.6f Rel)\n", mxperr,
		mxperr / (2.0 * (1<<(PW-1))));
	printf("Max magnitude error: %9.6f, expect %.2f\n", mxverr,
		2.0 * sqrt(QUANTIZATION_VARIANCE));
	printf("Avg phase err:       %9.6f, expect %.2f\n", sqrt(sum_perr),
		sqrt(PHASE_VARIANCE_RAD) * RAD_TO_PHASE);

	if (failed_test) {
		printf("TEST FAILED!!\n");
		exit(EXIT_FAILURE);
	} else {
		printf("SUCCESS\n");
		exit(EXIT_SUCCESS);
	}
}
